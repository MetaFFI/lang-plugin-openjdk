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
	code, err := this.generateCode()
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
	}
	
	file, err := this.buildDynamicLibrary(code)
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
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
func (this *HostCompiler) parseForeignStubs() (string, error) {
	
	return TemplateFunctions2.RunTemplate("HostFunctionStubsTemplate", HostFunctionStubsTemplate, this.def, templatesFuncMap)
	
}

//--------------------------------------------------------------------
func (this *HostCompiler) generateCode() (string, error) {
	
	header, err := this.parseHeader()
	if err != nil {
		return "", err
	}
	
	functionStubs, err := this.parseForeignStubs()
	if err != nil {
		return "", err
	}
	
	res := header + HostPackage + HostImports + functionStubs
	
	return res, nil
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
func (this *HostCompiler) buildDynamicLibrary(code string) ([]byte, error) {
	
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
	
	javaFiles := make([]string, 0)
	for _, m := range this.def.Modules {
		javaFilename := dir + m.Name + ".java"
		javaFiles = append(javaFiles, javaFilename)
		err = ioutil.WriteFile(javaFilename, []byte(code), 0700)
		if err != nil {
			return nil, fmt.Errorf("Failed to write host java code: %v", err)
		}
		
		println(code)
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
	output, err := buildCmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("Failed compiling host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}
	
	classFiles, err := filepath.Glob(dir + "metaffi" + string(os.PathSeparator) + "*.class")
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
	result, err := ioutil.ReadFile(dir + this.def.IDLFilename + ".jar")
	if err != nil {
		return nil, fmt.Errorf("Failed to read host OpenJDK runtime linker at: %v. Error: %v", this.def.IDLFilename+"_MetaFFIHost.jar", err)
	}
	
	return result, nil
}

//--------------------------------------------------------------------
