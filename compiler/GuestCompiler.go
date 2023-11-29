package main

import (
	"fmt"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"github.com/yargevad/filepathx"
	"io/ioutil"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"runtime"
	"strings"
	"text/template"
)

// --------------------------------------------------------------------
type GuestCompiler struct {
	def            *IDL.IDLDefinition
	outputDir      string
	outputFilename string
}

// --------------------------------------------------------------------
func NewGuestCompiler() *GuestCompiler {
	return &GuestCompiler{}
}

// --------------------------------------------------------------------
func getDynamicLibSuffix() string {
	switch runtime.GOOS {
	case "windows":
		return ".dll"
	case "darwin":
		return ".dylib"
	default: // We might need to make this more specific in the future
		return ".so"
	}
}

func (this *GuestCompiler) replaceIllegalCharacters() {
	for _, m := range this.def.Modules {
		m.Name = strings.ReplaceAll(m.Name, ".", "_")
		m.Name = strings.ReplaceAll(m.Name, "-", "_")

		for _, c := range m.Classes {
			c.Name = strings.ReplaceAll(c.Name, ".", "_")
			c.Name = strings.ReplaceAll(c.Name, "-", "_")

			for _, f := range c.Methods {
				f.Name = strings.ReplaceAll(f.Name, ".", "_")
				f.Name = strings.ReplaceAll(f.Name, "-", "_")
			}

			for _, cstr := range c.Constructors {
				cstr.Name = strings.ReplaceAll(cstr.Name, ".", "_")
				cstr.Name = strings.ReplaceAll(cstr.Name, "-", "_")
			}

			if c.Releaser != nil {
				c.Releaser.Name = strings.ReplaceAll(c.Releaser.Name, ".", "_")
				c.Releaser.Name = strings.ReplaceAll(c.Releaser.Name, "-", "_")
			}
		}
	}
}

// --------------------------------------------------------------------
func (this *GuestCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, guestOptions map[string]string) (err error) {

	if outputFilename == "" {
		outputFilename = definition.IDLFilename
	}

	this.def = definition
	this.outputDir = outputDir
	this.outputFilename = outputFilename

	this.replaceIllegalCharacters()

	// generate code
	jarcode, err := this.generateJarCode()
	if err != nil {
		return fmt.Errorf("Failed to generate guest jar code: %v", err)
	}

	entrypointCPPcode, err := this.generateEntrypointCPPCode()
	if err != nil {
		return fmt.Errorf("Failed to generate guest C++ code: %v", err)
	}

	jarfile, err := this.buildJar(jarcode, definition, outputDir, guestOptions)
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
	}

	dynamicLibraryFile, err := this.buildDynamicLibrary(entrypointCPPcode)
	if err != nil {
		return fmt.Errorf("Failed to generate guest code: %v", err)
	}

	// write to output
	outputFullFileName := fmt.Sprintf("%v%v%v_MetaFFIGuest.jar", this.outputDir, string(os.PathSeparator), this.outputFilename)
	err = ioutil.WriteFile(outputFullFileName, jarfile, 0700)
	if err != nil {
		return fmt.Errorf("Failed to write dynamic library to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}

	outputFullFileName = fmt.Sprintf("%v%v%v_MetaFFIGuest%v", this.outputDir, string(os.PathSeparator), this.outputFilename, getDynamicLibSuffix())
	err = ioutil.WriteFile(outputFullFileName, dynamicLibraryFile, 0700)
	if err != nil {
		return fmt.Errorf("Failed to write dynamic library to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}

	return nil

}

// --------------------------------------------------------------------
func (this *GuestCompiler) generateEntrypointCPPCode() (string, error) {
	tmpEntryPoint, err := template.New("GuestCPPEntrypoint").Funcs(templatesFuncMap).Parse(GuestCPPEntrypoint)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestCPPEntrypoint: %v", err)
	}

	bufEntryPoint := strings.Builder{}
	err = tmpEntryPoint.Execute(&bufEntryPoint, this.def)

	return bufEntryPoint.String(), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseHeader() (string, error) {
	tmp, err := template.New("GuestHeaderTemplate").Parse(GuestHeaderTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestHeaderTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)

	return buf.String(), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseImports() (string, error) {

	// imports.Imports contains all the imports.

	tmp, err := template.New("GuestImportsTemplate").Funcs(templatesFuncMap).Parse(GuestImportsTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestImportsTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)
	importsCode := buf.String()

	return importsCode, err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) parseForeignFunctions() (string, error) {

	tmpEntryPoint, err := template.New("GuestFunctionXLLRTemplate").Funcs(templatesFuncMap).Parse(GuestFunctionXLLRTemplate)
	if err != nil {
		return "", fmt.Errorf("Failed to parse GuestFunctionXLLRTemplate: %v", err)
	}

	bufEntryPoint := strings.Builder{}
	err = tmpEntryPoint.Execute(&bufEntryPoint, this.def)

	return bufEntryPoint.String(), err
}

// --------------------------------------------------------------------
func (this *GuestCompiler) generateJarCode() (string, error) {

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

	return res, nil
}

// --------------------------------------------------------------------
func (this *GuestCompiler) getClassPath(guestOptions map[string]string) string {

	classPathSet := make(map[string]bool, 0)
	wd, err := os.Getwd()
	if err != nil {
		panic(err)
	}
	classPathSet[wd] = true
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

	if classPathOption, exists := guestOptions["classPath"]; exists {
		classPath = append(classPath, strings.Split(classPathOption, ",")...)
	}

	return strings.Join(classPath, string(os.PathListSeparator))
}

// --------------------------------------------------------------------
func (this *GuestCompiler) buildDynamicLibrary(code string) ([]byte, error) {

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

	// write generated code to temp folder
	cppFiles := make([]string, 0)
	for _, m := range this.def.Modules {
		cppFilename := dir + m.Name + ".cpp"
		cppFiles = append(cppFiles, cppFilename)
		err = ioutil.WriteFile(cppFilename, []byte(code), 0700)
		if err != nil {
			return nil, fmt.Errorf("Failed to write host java code: %v", err)
		}
	}

	fmt.Println("Building Binary Entrypoint guest code")

	javaHome := os.Getenv("JAVA_HOME")
	if javaHome == "" {
		return nil, fmt.Errorf("JAVA_HOME is not set")
	}

	mffiHome := os.Getenv("METAFFI_HOME")
	if mffiHome == "" {
		return nil, fmt.Errorf("METAFFI_HOME is not set")
	}

	// compile generated java code
	args := make([]string, 0)
	args = append(args, "-o")
	args = append(args, dir+this.def.IDLFilename+getDynamicLibSuffix())
	args = append(args, "-I")
	args = append(args, mffiHome+"/include")
	args = append(args, "-I")
	args = append(args, javaHome+"/include")

	if runtime.GOOS == "windows" {
		args = append(args, "-I")
		args = append(args, javaHome+"/include/win32")
	}

	args = append(args, "-I")
	args = append(args, javaHome+"/include/"+runtime.GOOS)
	args = append(args, "-std=c++17")
	args = append(args, "-fPIC")
	args = append(args, "-shared")
	args = append(args, cppFiles...)
	buildCmd := exec.Command("g++", args...)
	fmt.Printf("Compiling %v\n", strings.Join(buildCmd.Args, " "))
	output, err := buildCmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("Failed compiling dynamic library entrypoints for OpenJDK guest. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}

	// read jar file and return
	result, err := ioutil.ReadFile(dir + this.def.IDLFilename + getDynamicLibSuffix())
	if err != nil {
		return nil, fmt.Errorf("Failed to read dynamic library entrypoints for OpenJDK guest %v. Error: %v", this.def.IDLFilename, err)
	}

	return result, nil
}

// --------------------------------------------------------------------
func (this *GuestCompiler) buildJar(code string, definition *IDL.IDLDefinition, outputDir string, guestOptions map[string]string) ([]byte, error) {

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
	// write generated code to temp folder
	javaFiles := make([]string, 0)
	for _, m := range this.def.Modules {
		javaFilename := dir + m.Name + "_Entrypoints.java"
		javaFiles = append(javaFiles, javaFilename)
		err = ioutil.WriteFile(javaFilename, []byte(code), 0700)
		if err != nil {
			return nil, fmt.Errorf("Failed to write host java code: %v", err)
		}
	}

	if javaFilesOption, exists := guestOptions["compileJavaFiles"]; exists {
		javaFiles = append(javaFiles, strings.Split(javaFilesOption, ",")...)
	}

	// if IDL is a java file
	if strings.ToLower(path.Ext(definition.IDLFilenameWithExtension)) == ".java" {
		javaFiles = append(javaFiles, outputDir+string(os.PathSeparator)+definition.IDLFilenameWithExtension)
	}

	fmt.Println("Building OpenJDK guest code")

	// compile generated java code
	args := make([]string, 0)
	args = append(args, "-d")
	args = append(args, dir)
	args = append(args, "-cp")

	args = append(args, this.getClassPath(guestOptions))

	args = append(args, javaFiles...)
	buildCmd := exec.Command("javac", args...)
	fmt.Printf("%v\n", strings.Join(buildCmd.Args, " "))

	output, err := buildCmd.CombinedOutput()

	if err != nil {
		return nil, fmt.Errorf("Failed compiling host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}

	classFiles, err := filepathx.Glob(dir + "**" + string(os.PathSeparator) + "*.class")
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
