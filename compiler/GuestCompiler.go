package main

import (
	"fmt"
	compiler "github.com/OpenFFI/plugin-sdk/compiler/go"
	"html/template"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
)

//--------------------------------------------------------------------
type GuestCompiler struct{
	def *compiler.IDLDefinition
	outputDir string
	serializationCode map[string]string
	outputFilename string
}
//--------------------------------------------------------------------
func NewGuestCompiler(definition *compiler.IDLDefinition, outputDir string, outputFilename string, serializationCode map[string]string) *GuestCompiler{

	serializationCodeCopy := make(map[string]string)
	for k, v := range serializationCode{
		serializationCodeCopy[k] = v
	}

	return &GuestCompiler{def: definition, outputDir: outputDir, serializationCode: serializationCodeCopy, outputFilename: outputFilename}
}
//--------------------------------------------------------------------
func (this *GuestCompiler) Compile() (outputFileName string, err error){

	// generate code
	code, err := this.generateCode()
	if err != nil{
		return "", fmt.Errorf("Failed to generate guest code: %v", err)
	}

	file, err := this.buildDynamicLibrary(code)
	if err != nil{
		return "", fmt.Errorf("Failed to generate guest code: %v", err)
	}

	// write to output
	outputFullFileName := fmt.Sprintf("%v%v%v_OpenFFIGuest.jar", this.outputDir, string(os.PathSeparator), this.outputFilename)
	err = ioutil.WriteFile(outputFullFileName, file, 0700)
	if err != nil{
		return "", fmt.Errorf("Failed to write dynamic library to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}

	return outputFullFileName, nil

}
//--------------------------------------------------------------------
func (this *GuestCompiler) parseHeader() (string, error){
	tmp, err := template.New("guest").Parse(GuestHeaderTemplate)
	if err != nil{
		return "", fmt.Errorf("Failed to parse GuestHeaderTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)

	return buf.String(), err
}
//--------------------------------------------------------------------
func (this *GuestCompiler) parseImports() (string, error){

	// get all imports from the def file
	imports := struct {
		Imports []string
	}{
		Imports: make([]string, 0),
	}

	set := make(map[string]bool)

	for _, m := range this.def.Modules{
		for _, f := range m.Functions{
			if pack, found := f.PathToForeignFunction["package"]; found{

				if pack != `openffi`{
					set[pack] = true
				}
			}
		}
	}

	for k, _ := range set{
		imports.Imports = append(imports.Imports, k)
	}

	// imports.Imports contains all the imports.

	tmp, err := template.New("guest").Parse(GuestImportsTemplate)
	if err != nil{
		return "", fmt.Errorf("Failed to parse GuestImportsTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, imports)
	importsCode := buf.String()

	/*
	// get all imports/includes from the serialization code
	for filename, code := range this.serializationCode{
		// the serialization code files
		if strings.ToLower(filepath.Ext(filename)) != ".java"{
			continue
		}

		reImports := regexp.MustCompile(`import[ ]+([^;]+;)`)
		serializationImports := reImports.FindAllString(code, -1)
		for _, imp := range serializationImports{
			importsCode += "import "+imp+"\n"
		}
	}
	*/

	return importsCode, err
}
//--------------------------------------------------------------------
func (this *GuestCompiler) parseForeignFunctions() (string, error){

	// Key: Function name, Val: Function itself
	// Function signature: func(string)string or func(interface{})string
	funcMap := map[string]interface{}{
		"Title": func(elem string)string{
			return strings.Title(elem)
		},

		"ToJavaType": func (elem string) string{
			pyType, found := OpenFFITypeToJavaType[elem]
			if !found{
				panic("Type "+elem+" is not an OpenFFI type")
			}

			return pyType
		},

		"ToParamCall": func (paramObj string, fieldObj interface{}) string{
			field := fieldObj.(*compiler.FieldDefinition)

			jType, found := OpenFFITypeToJavaType[field.Type]
			if !found{
				panic("Type "+field.Type+" is not an OpenFFI type")
			}

			if field.IsArray{
				return fmt.Sprintf("%v.get%vList().toArray(new %v[0])", paramObj, strings.Title(field.Name), jType)
			} else {
				return fmt.Sprintf("%v.get%v()", paramObj, strings.Title(field.Name))
			}
		},
	}

	tmpEntryPoint, err := template.New("guest").Funcs(funcMap).Parse(GuestFunctionXLLRTemplate)
	if err != nil{
		return "", fmt.Errorf("Failed to parse GuestFunctionXLLRTemplate: %v", err)
	}

	bufEntryPoint := strings.Builder{}
	err = tmpEntryPoint.Execute(&bufEntryPoint, this.def)

	return bufEntryPoint.String(), err
}
//--------------------------------------------------------------------
func (this *GuestCompiler) generateCode() (string, error){

	header, err := this.parseHeader()
	if err != nil{ return "", err }

	imports, err := this.parseImports()
	if err != nil{ return "", err }

	functionStubs, err := this.parseForeignFunctions()
	if err != nil{ return "", err }

	res := header + GuestPackage + imports + functionStubs

	// append serialization code in the same file
	for filename, serializationCode := range this.serializationCode{

        // the serialization code files
        if strings.ToLower(filepath.Ext(filename)) != ".java"{
            continue
        }

		rePackage := regexp.MustCompile(`package[ ]+[^;]+;`)
		serializationCode = rePackage.ReplaceAllString(serializationCode, "")
		serializationCode = strings.Replace(serializationCode, "public final class", "final class", -1)

		res += serializationCode
	}

	return res, nil
}
//--------------------------------------------------------------------
func (this *GuestCompiler) getClassPath() string{

	classPathSet := make(map[string]bool, 0)
	classPathSet["."] = true
	classPathSet[fmt.Sprintf("%v%vprotobuf-java-3.15.2.jar", os.Getenv("OPENFFI_HOME"), string(os.PathSeparator))] = true
	classPathSet[fmt.Sprintf("%v%vxllr.openjdk.bridge.jar", os.Getenv("OPENFFI_HOME"), string(os.PathSeparator))] = true

	classPath := make([]string, 0)
	for k, _ := range classPathSet{
		classPath = append(classPath, k)
	}

	for _, m := range this.def.Modules{
		for _, f := range m.Functions{
			if cp, found := f.PathToForeignFunction["classpath"]; found{
				classPath = append(classPath, os.ExpandEnv(cp))
			}
		}
	}

	return strings.Join(classPath, string(os.PathListSeparator))
}
//--------------------------------------------------------------------
func (this *GuestCompiler) buildDynamicLibrary(code string)([]byte, error){

	dir, err := os.MkdirTemp("", "openffi_openjdk_compiler*")
	if err != nil{
		return nil, fmt.Errorf("Failed to create temp dir to build code: %v", err)
	}
	defer func(){ if err == nil{ _ = os.RemoveAll(dir) } }()

	dir = dir+string(os.PathSeparator)

	// write generated code to temp folder
	javaFiles := make([]string, 0)
	for _, m := range this.def.Modules{
		javaFilename := dir+m.Name+".java"
		javaFiles = append(javaFiles, javaFilename)
		err = ioutil.WriteFile(javaFilename, []byte(code), 0700)
		if err != nil{
			return nil, fmt.Errorf("Failed to write host java code: %v", err)
		}
	}

	fmt.Println("Building OpenJDK host code")

	// compile generated java code
	args := make([]string, 0)
	args = append(args, "-d")
	args = append(args, dir)
	args = append(args, "-cp")
	args = append(args, this.getClassPath())
	args = append(args, javaFiles...)
	buildCmd := exec.Command("javac", args...)
	fmt.Printf("%v\n", strings.Join(buildCmd.Args, " "))
	output, err := buildCmd.CombinedOutput()
	if err != nil{
		return nil, fmt.Errorf("Failed compiling host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}

	classFiles, err := filepath.Glob(dir+"openffi"+string(os.PathSeparator)+"*.class")
	if err != nil{
		return nil, fmt.Errorf("Failed to get list of class files to compile in the path %v*.class: %v", "openffi", err)
	}

	for i, file := range classFiles{
		classFiles[i], err = filepath.Rel(dir, file)
		if err != nil{
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
	if err != nil{
		return nil, fmt.Errorf("Failed building jar for host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}

	// read jar file and return
	result, err := ioutil.ReadFile(dir+this.def.IDLFilename+".jar")
	if err != nil{
		return nil, fmt.Errorf("Failed to read host OpenJDK runtime linker %v. Error: %v", this.def.IDLFilename, err)
	}

	return result, nil
}
//--------------------------------------------------------------------

