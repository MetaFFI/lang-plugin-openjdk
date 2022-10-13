package main

import (
	"fmt"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"html/template"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
)

//--------------------------------------------------------------------
type GuestCompiler struct {
	def               *IDL.IDLDefinition
	outputDir         string
	serializationCode map[string]string
	outputFilename    string
	blockName         string
	blockCode         string
}

//--------------------------------------------------------------------
func NewGuestCompiler() *GuestCompiler {
	return &GuestCompiler{}
}

//--------------------------------------------------------------------
func (this *GuestCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, blockName string, blockCode string) (err error) {
	
	if outputFilename == "" {
		outputFilename = definition.IDLFilename
	}
	
	if strings.Contains(outputFilename, "#") {
		toRemove := outputFilename[strings.LastIndex(outputFilename, string(os.PathSeparator))+1 : strings.Index(outputFilename, "#")+1]
		outputFilename = strings.ReplaceAll(outputFilename, toRemove, "")
	}
	
	this.def = definition
	this.outputDir = outputDir
	this.blockName = blockName
	this.blockCode = blockCode
	this.outputFilename = outputFilename
	
	// generate code
	code, err := this.generateCode()
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
	}
	
	file, err := this.buildDynamicLibrary(code)
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
	}
	
	// write to output
	outputFullFileName := fmt.Sprintf("%v%v%v_MetaFFIGuest.jar", this.outputDir, string(os.PathSeparator), this.outputFilename)
	err = ioutil.WriteFile(outputFullFileName, file, 0700)
	if err != nil {
		return fmt.Errorf("Failed to write dynamic library to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}
	
	return nil
	
}

//--------------------------------------------------------------------
func (this *GuestCompiler) parseHeader() (string, error) {
	tmp, err := template.New("guest").Parse(GuestHeaderTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestHeaderTemplate: %v", err)
	}
	
	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)
	
	return buf.String(), err
}

//--------------------------------------------------------------------
func (this *GuestCompiler) parseImports() (string, error) {

	// imports.Imports contains all the imports.
	
	tmp, err := template.New("guest").Funcs(templatesFuncMap).Parse(GuestImportsTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestImportsTemplate: %v", err)
	}
	
	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)
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
func (this *GuestCompiler) parseForeignFunctions() (string, error) {
	
	tmpEntryPoint, err := template.New("guest").Funcs(templatesFuncMap).Parse(GuestFunctionXLLRTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestFunctionXLLRTemplate: %v", err)
	}
	
	bufEntryPoint := strings.Builder{}
	err = tmpEntryPoint.Execute(&bufEntryPoint, this.def)
	
	return bufEntryPoint.String(), err
}

//--------------------------------------------------------------------
func (this *GuestCompiler) generateCode() (string, error) {
	
	header, err := this.parseHeader()
	if err != nil {
		return "", err
	}
	
	imports, err := this.parseImports()
	if err != nil {
		return "", err
	}
	
	functionStubs, err := this.parseForeignFunctions()
	if err != nil {
		return "", err
	}
	
	res := header + GuestPackage + imports + functionStubs
	
	// append serialization code in the same file
	for filename, serializationCode := range this.serializationCode {
		
		// the serialization code files
		if strings.ToLower(filepath.Ext(filename)) != ".java" {
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
func (this *GuestCompiler) getClassPath() string {
	
	classPathSet := make(map[string]bool, 0)
	classPathSet["."] = true
	classPathSet[fmt.Sprintf("%v%vxllr.openjdk.bridge.jar", os.Getenv("METAFFI_HOME"), string(os.PathSeparator))] = true
	
	for _, m := range this.def.Modules {
		for _, r := range m.ExternalResources {
			classpathResource := os.ExpandEnv(r)
			classPathSet[classpathResource] = true
		}
	}
	
	classPath := make([]string, 0)
	for k, _ := range classPathSet {
		classPath = append(classPath, k)
	}
	
	return strings.Join(classPath, string(os.PathListSeparator))
}

//--------------------------------------------------------------------
func (this *GuestCompiler) buildDynamicLibrary(code string) ([]byte, error) {
	
	dir, err := os.MkdirTemp("", "metaffi_openjdk_compiler*")
	if err != nil {
		return nil, fmt.Errorf("Failed to create temp dir to build code: %v", err)
	}
	defer func() {
		if err == nil {
			_ = os.RemoveAll(dir)
		}
	}()
	
	dir = dir + string(os.PathSeparator)
	
	// write generated code to temp folder
	javaFiles := make([]string, 0)
	for _, m := range this.def.Modules {
		javaFilename := dir + m.Name + ".java"
		javaFiles = append(javaFiles, javaFilename)
		err = ioutil.WriteFile(javaFilename, []byte(code), 0700)
		if err != nil {
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
	if err != nil {
		return nil, fmt.Errorf("Failed compiling host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}
	
	classFiles, err := filepath.Glob(dir + "metaffi_guest" + string(os.PathSeparator) + "*.class")
	if err != nil {
		return nil, fmt.Errorf("Failed to get list of class files to compile in the path %v*.class: %v", "metaffi_guest", err)
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
	result, err := ioutil.ReadFile(dir + this.def.IDLFilename + ".jar")
	if err != nil {
		return nil, fmt.Errorf("Failed to read host OpenJDK runtime linker %v. Error: %v", this.def.IDLFilename, err)
	}
	
	return result, nil
}

//--------------------------------------------------------------------
