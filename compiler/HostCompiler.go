package main

import (
	"fmt"
	compiler "github.com/MetaFFI/plugin-sdk/compiler/go"
	TemplateFunctions2 "github.com/MetaFFI/plugin-sdk/compiler/go/CodeTemplates"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
)

var javaKeywords = map[string]bool{
	"abstract":     true,
	"assert":       true,
	"boolean":      true,
	"break":        true,
	"byte":         true,
	"case":         true,
	"catch":        true,
	"char":         true,
	"class":        true,
	"const":        true,
	"continue":     true,
	"default":      true,
	"do":           true,
	"double":       true,
	"else":         true,
	"enum":         true,
	"extends":      true,
	"final":        true,
	"finally":      true,
	"float":        true,
	"for":          true,
	"goto":         true,
	"if":           true,
	"implements":   true,
	"import":       true,
	"instanceof":   true,
	"int":          true,
	"interface":    true,
	"long":         true,
	"native":       true,
	"new":          true,
	"package":      true,
	"private":      true,
	"protected":    true,
	"public":       true,
	"return":       true,
	"short":        true,
	"static":       true,
	"strictfp":     true,
	"super":        true,
	"switch":       true,
	"synchronized": true,
	"this":         true,
	"throw":        true,
	"throws":       true,
	"transient":    true,
	"try":          true,
	"void":         true,
	"volatile":     true,
	"while":        true,
}

// --------------------------------------------------------------------
type HostCompiler struct {
	def            *IDL.IDLDefinition
	outputDir      string
	hostOptions    map[string]string
	outputFilename string
}

// --------------------------------------------------------------------
func NewHostCompiler() *HostCompiler {
	return &HostCompiler{}
}

// --------------------------------------------------------------------
func fixIdenticalFunctions(def *IDL.IDLDefinition) {
	// each element is an array of overloaded functions

	setOfFunctions := make(map[string][]*IDL.FunctionDefinition)

	for _, m := range def.Modules {
		for _, f := range m.Functions {

			sigStr := f.Name
			for _, p := range f.Parameters {
				sigStr += string(p.Type)
			}

			_, found := setOfFunctions[sigStr]
			if !found {
				setOfFunctions[sigStr] = make([]*IDL.FunctionDefinition, 0)
			}

			setOfFunctions[sigStr] = append(setOfFunctions[sigStr], f)
		}

		for _, c := range m.Classes {
			for _, m := range c.Methods {
				sigStr := c.Name + m.Name
				for _, p := range m.Parameters {
					sigStr += string(p.Type)
				}

				_, found := setOfFunctions[sigStr]
				if !found {
					setOfFunctions[sigStr] = make([]*IDL.FunctionDefinition, 0)
				}

				setOfFunctions[sigStr] = append(setOfFunctions[sigStr], &m.FunctionDefinition)
			}
		}
	}

	// for each identical function/method, append index to it
	for _, identicalFunctions := range setOfFunctions {
		if len(identicalFunctions) > 1 {
			for i, identicalFunc := range identicalFunctions {
				if i == 0 {
					continue
				}

				identicalFunc.Name += strconv.Itoa(i)
			}
		}
	}
}

// --------------------------------------------------------------------
func mergeIdenticalConstructors(def *IDL.IDLDefinition) {
	// in case there are multiple constructors that have the same signature:
	// 1. leave only one implementation
	// 2. add additional parameter for the "ID" - use that to call the foreign entity
	// 3. write in the comments for each overload "comments of function \n signature - ID", so the user can pass the correct ID
	for _, m := range def.Modules {
		for _, c := range m.Classes {
			if len(c.Constructors) > 1 {
				setOfConstructors := make(map[string][]*IDL.ConstructorDefinition)

				for _, cstr := range c.Constructors {
					sigStr := ""
					for _, p := range cstr.Parameters {
						sigStr += string(p.Type)
					}

					_, found := setOfConstructors[sigStr]
					if !found {
						setOfConstructors[sigStr] = make([]*IDL.ConstructorDefinition, 0)
					}

					setOfConstructors[sigStr] = append(setOfConstructors[sigStr], cstr)
				}

				// merge identicals
				generateConstructorComment := func(curcstr *IDL.ConstructorDefinition) string {
					comment := "Constructor ID: " + curcstr.GetEntityIDName() + "\n"
					comment += "Parameters Names: "
					for _, p := range curcstr.Parameters {
						comment += p.Name + " "
					}

					if curcstr.Comment != "" {
						comment += "\n" + curcstr.Comment
					}

					comment += "\n\n"
					return comment
				}

				constructorsToRemove := make([]*IDL.ConstructorDefinition, 0)
				for _, identicalConstructors := range setOfConstructors {
					if len(identicalConstructors) > 1 { // need to merge
						identicalConstructors[0].Tags["AddFunctionIDParameter"] = ""

						identicalConstructors[0].Comment = generateConstructorComment(identicalConstructors[0])

						for i := 1; i < len(identicalConstructors); i++ {
							identicalConstructors[0].Comment += generateConstructorComment(identicalConstructors[i])
							constructorsToRemove = append(constructorsToRemove, identicalConstructors[i])
						}
					}
				}

				for _, cstrToRemove := range constructorsToRemove {
					for i, cstr := range c.Constructors {
						if cstrToRemove == cstr {

							// remove item
							if i+1 == len(c.Constructors) { // if last item
								c.Constructors = c.Constructors[:i]
							} else { // if in the middle
								c.Constructors = append(c.Constructors[:i], c.Constructors[i+1:]...)
							}
						}
					}
				}
			}
		}
	}

}

// --------------------------------------------------------------------
func fixModuleNameIfMatchesToClass(def *IDL.IDLDefinition) {

	// if module name == class name,
	// the generated java class for the module, holding all the IDs
	// overwrites the java class for the class.
	// To fix this, in such a case, rename the module to "[module name]Module"

	for _, m := range def.Modules {
		for _, c := range m.Classes {
			if strings.ToLower(m.Name) == strings.ToLower(c.Name) {
				m.Name += "Module"
				return
			}
		}
	}
}

// --------------------------------------------------------------------
func handleOptionalParameters(def *IDL.IDLDefinition) {
	// turn default parameters into overloaded callables

	for _, mod := range def.Modules {
		functions, methods, constructors := mod.GetCallablesWithOptionalParameters(true, true, true)

		for _, f := range functions {
			firstIndexOfOptionalParameter := f.GetFirstIndexOfOptionalParameter()

			var j int32 = 0
			for i := firstIndexOfOptionalParameter; i < len(f.Parameters)-1; i++ {
				j += 1
				dup := f.Duplicate()
				dup.OverloadIndex = j
				dup.Parameters = dup.Parameters[:i]
				mod.Functions = append(mod.Functions, dup)
			}
		}

		for _, cstr := range constructors {
			firstIndexOfOptionalParameter := cstr.GetFirstIndexOfOptionalParameter()

			var j int32 = 0
			for i := firstIndexOfOptionalParameter; i < len(cstr.Parameters)-1; i++ {
				j += 1
				dup := cstr.Duplicate()
				dup.OverloadIndex = j
				dup.Parameters = dup.Parameters[:i]
				cstr.GetParent().AddConstructor(dup)
			}
		}

		for _, m := range methods {
			firstIndexOfOptionalParameter := m.GetFirstIndexOfOptionalParameter()

			var j int32 = 0
			for i := firstIndexOfOptionalParameter; i < len(m.Parameters); i++ {
				j += 1
				dup := m.Duplicate()
				dup.OverloadIndex = j
				dup.Parameters = dup.Parameters[:i]
				m.GetParent().AddMethod(dup)
			}
		}
	}
}

// --------------------------------------------------------------------
func (this *HostCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, hostOptions map[string]string) (err error) {

	compiler.ModifyKeywords(definition, javaKeywords, func(keyword string) string { return keyword + "__" })
	mergeIdenticalConstructors(definition)
	fixIdenticalFunctions(definition)
	fixModuleNameIfMatchesToClass(definition)
	handleOptionalParameters(definition)

	this.def = definition
	this.outputDir = outputDir
	this.hostOptions = hostOptions
	this.outputFilename = outputFilename

	// generate code
	codefiles, err := this.generateCode()
	if err != nil {
		return fmt.Errorf("Failed to generate host code: %v", err)
	}

	file, err := this.buildDynamicLibrary(codefiles)
	if err != nil {
		return fmt.Errorf("Failed to generate host code: %v", err)
	}

	// write to output
	outputFilename = this.outputDir + string(os.PathSeparator) + this.outputFilename + "_MetaFFIHost.jar"
	err = ioutil.WriteFile(outputFilename, file, 0600)
	if err != nil {
		return fmt.Errorf("Failed to write host code to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}
	if err != nil {
		return fmt.Errorf("Failed to write host code to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}

	return nil

}

// --------------------------------------------------------------------
func (this *HostCompiler) parseHeader() (string, error) {
	return TemplateFunctions2.RunTemplate("parseHeader", HostHeaderTemplate, this.def)
}

// --------------------------------------------------------------------
func (this *HostCompiler) parseForeignStubs() (map[string]string, error) {

	res := make(map[string]string)

	if len(this.def.Modules) > 1 {
		panic("OpenJDK plugin currently does not support multiple modules. Split modules into separate IDLs")
	}

	// place in module file
	modfile, err := TemplateFunctions2.RunTemplate("OpenJDK HostFunctionStubsTemplate", HostFunctionStubsTemplate, this.def, templatesFuncMap)
	if err != nil {
		return nil, err
	}

	res[this.def.Modules[0].Name+".java"] = modfile

	for _, m := range this.def.Modules {
		for _, c := range m.Classes {

			tempParam := struct {
				C *IDL.ClassDefinition
				M *IDL.ModuleDefinition
			}{
				C: c,
				M: m,
			}

			res[c.Name+".java"], err = TemplateFunctions2.RunTemplate("OpenJDK HostClassesStubsTemplate", HostClassesStubsTemplate, tempParam, templatesFuncMap)

			if err != nil {
				return nil, err
			}
		}
	}

	return res, nil

}

// --------------------------------------------------------------------
func (this *HostCompiler) generateCode() (map[string]string, error) {

	header, err := this.parseHeader()
	if err != nil {
		return nil, err
	}

	functionStubs, err := this.parseForeignStubs()
	if err != nil {
		return nil, err
	}

	for filename, code := range functionStubs {
		functionStubs[filename] = header + HostPackage + HostImports + code
	}

	return functionStubs, nil
}

// --------------------------------------------------------------------
func (this *HostCompiler) getClassPath() string {

	classPath := make([]string, 0)
	classPath = append(classPath, ".")
	classPath = append(classPath, fmt.Sprintf("%v%v", os.Getenv("METAFFI_HOME"), string(os.PathSeparator)))
	classPath = append(classPath, fmt.Sprintf("%v%vxllr.openjdk.bridge.jar", os.Getenv("METAFFI_HOME"), string(os.PathSeparator)))

	return strings.Join(classPath, string(os.PathListSeparator))
}

// --------------------------------------------------------------------
func (this *HostCompiler) buildDynamicLibrary(codefiles map[string]string) ([]byte, error) {

	dir, err := os.MkdirTemp("", "metaffi_openjdk_compiler*")
	if err != nil {
		return nil, fmt.Errorf("Failed to create temp dir to build code: %v", err)
	}
	defer func() {
		if err == nil {
			//_ = os.RemoveAll(dir)
		}
	}()

	dir = dir + string(os.PathSeparator)

	javaFiles := make([]string, 0)
	for filename, code := range codefiles { // TODO: handle multiple modules
		javaFiles = append(javaFiles, filename)
		err = ioutil.WriteFile(filename, []byte(code), 0700)
		if err != nil {
			return nil, fmt.Errorf("Failed to write host java code: %v", err)
		}

		println("writing file " + filename)
	}

	fmt.Println("Building OpenJDK host code")

	// compile java code
	args := make([]string, 0)
	args = append(args, "-d")
	args = append(args, dir)
	args = append(args, "-cp")
	args = append(args, this.getClassPath())
	args = append(args, javaFiles...)
	buildCmd := exec.Command("javac", args...)
	fmt.Printf("%v\n", strings.Join(buildCmd.Args, " "))
	var output []byte
	output, err = buildCmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("Failed compiling host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}

	var classFiles []string
	classFiles, err = filepath.Glob(dir + "metaffi_host" + string(os.PathSeparator) + "*.class")
	if err != nil {
		return nil, fmt.Errorf("Failed to get list of class files to compile in the path %v*.class: %v", "metaffi", err)
	}

	for i, file := range classFiles {
		classFiles[i], err = filepath.Rel(dir, file)
		if err != nil {
			return nil, err
		}
	}

	// jar all class files
	args = make([]string, 0)
	args = append(args, "cf")
	args = append(args, this.def.IDLFilename+".jar")
	args = append(args, classFiles...)
	buildCmd = exec.Command("jar", args...)
	buildCmd.Dir = dir
	fmt.Printf("%v\n", strings.Join(buildCmd.Args, " "))
	output, err = buildCmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("Failed building jar for host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}

	// read jar file and return
	var result []byte
	result, err = ioutil.ReadFile(dir + this.def.IDLFilename + ".jar")
	if err != nil {
		return nil, fmt.Errorf("Failed to read host OpenJDK runtime linker at: %v. Error: %v", this.def.IDLFilename+"_MetaFFIHost.jar", err)
	}

	// delete generated class files
	for _, file := range javaFiles {
		err := os.Remove(file)
		if err != nil {
			fmt.Printf("Failed to delete and cleanup %v - %v\n", file, err)
		}
	}

	return result, nil
}

//--------------------------------------------------------------------
