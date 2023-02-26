package main

import (
	"fmt"
	TemplateFunctions2 "github.com/MetaFFI/plugin-sdk/compiler/go/CodeTemplates"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

//--------------------------------------------------------------------
type HostCompiler struct {
	def            *IDL.IDLDefinition
	outputDir      string
	hostOptions    map[string]string
	outputFilename string
}

//--------------------------------------------------------------------
func NewHostCompiler() *HostCompiler {
	return &HostCompiler{}
}

//--------------------------------------------------------------------
func (this *HostCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, hostOptions map[string]string) (err error) {
	
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

//--------------------------------------------------------------------
func (this *HostCompiler) parseHeader() (string, error) {
	return TemplateFunctions2.RunTemplate("parseHeader", HostHeaderTemplate, this.def)
}

//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
func (this *HostCompiler) getClassPath() string {
	
	classPath := make([]string, 0)
	classPath = append(classPath, ".")
	classPath = append(classPath, fmt.Sprintf("%v%v", os.Getenv("METAFFI_HOME"), string(os.PathSeparator)))
	classPath = append(classPath, fmt.Sprintf("%v%vxllr.openjdk.bridge.jar", os.Getenv("METAFFI_HOME"), string(os.PathSeparator)))
	
	return strings.Join(classPath, string(os.PathListSeparator))
}

//--------------------------------------------------------------------
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
	
	return result, nil
}

//--------------------------------------------------------------------
