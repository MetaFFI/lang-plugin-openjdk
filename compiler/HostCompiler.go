package main

import (
	"fmt"
	compiler "github.com/MetaFFI/plugin-sdk/compiler/go"
	"html/template"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

//--------------------------------------------------------------------
type HostCompiler struct{
	def *compiler.IDLDefinition
	outputDir string
	serializationCode map[string]string
	hostOptions map[string]string
	outputFilename string
}
//--------------------------------------------------------------------
func NewHostCompiler(definition *compiler.IDLDefinition, outputDir string, outputFilename string, serializationCode map[string]string, hostOptions map[string]string) *HostCompiler{

	serializationCodeCopy := make(map[string]string)
	for k, v := range serializationCode{
		serializationCodeCopy[k] = v
	}

	return &HostCompiler{def: definition,
		outputDir: outputDir,
		serializationCode: serializationCodeCopy,
		outputFilename: outputFilename,
		hostOptions: hostOptions}
}
//--------------------------------------------------------------------
func (this *HostCompiler) Compile() (outputFileName string, err error){

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
	outputFileName = this.outputDir+string(os.PathSeparator)+this.outputFilename+"_MetaFFIHost.jar"
	err = ioutil.WriteFile( outputFileName, file, 0600)
	if err != nil{
		return "", fmt.Errorf("Failed to write host code to %v. Error: %v", this.outputDir+this.outputFilename, err)
	}

	return outputFileName, nil

}
//--------------------------------------------------------------------
func (this *HostCompiler) parseHeader() (string, error){
	tmp, err := template.New("host").Parse(HostHeaderTemplate)
	if err != nil{
		return "", fmt.Errorf("Failed to parse HostHeaderTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)

	return buf.String(), err
}
//--------------------------------------------------------------------
func (this *HostCompiler) parseImports() (string, error){

	/*
	importsCode := HostImports

	// get all imports from the serialization code

	for filename, _ := range this.serializationCode{

        // the serialization code files
        if strings.ToLower(filepath.Ext(filename)) != ".java"{
            continue
        }

	}

	return importsCode, nil
	*/

	return "", nil
}
//--------------------------------------------------------------------
func (this *HostCompiler) parseForeignStubs() (string, error){

	// Key: Function name, Val: Function itself
    // Function signature: func(string)string or func(interface{})string
    funcMap := map[string]interface{}{
    	"Title": func(elem string)string{
    		elem = strings.ReplaceAll(elem, "_", " ")
    		elem = strings.Title(elem)
    		return strings.ReplaceAll(elem, " ", "")
	    },

	    "ToJavaType": func (elem string) string{
		    pyType, found := MetaFFITypeToJavaType[elem]
		    if !found{
			    panic("Type "+elem+" is not an MetaFFI type")
		    }

		    return pyType
	    },
    }

	tmp, err := template.New("host").Funcs(funcMap).Parse(HostFunctionStubsTemplate)
	if err != nil{
		return "", fmt.Errorf("Failed to parse HostFunctionStubsTemplate: %v", err)
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, this.def)

	return buf.String(), err
}
//--------------------------------------------------------------------
func (this *HostCompiler) parsePackage() (string, error){

	/*
	tmp, err := template.New("host").Parse(HostPackageTemplate)
	if err != nil{
		return "", fmt.Errorf("Failed to parse HostFunctionStubsTemplate: %v", err)
	}

	PackageName := struct {
		Package string
	}{
		Package: "main",
	}

	// TODO: an example of updating the package based on "host option" key "package"
	// Modify to suit your needs
	if pckName, found := this.hostOptions["package"]; found{
		PackageName.Package = pckName
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, &PackageName)

	return buf.String(), err
	*/

	return "", nil
}
//--------------------------------------------------------------------
func (this *HostCompiler) generateCode() (string, error){

	header, err := this.parseHeader()
	if err != nil{ return "", err }

	//packageDeclaration, err := this.parsePackage()
	//if err != nil{
	//	return "", err
	//}

	//imports, err := this.parseImports()
	//if err != nil{ return "", err }

	functionStubs, err := this.parseForeignStubs()
	if err != nil{ return "", err }

	res := header + HostPackage + HostImports + functionStubs

	// append serialization code in the same file
	for filename, serializationCode := range this.serializationCode{

        // the serialization code files
        if strings.ToLower(filepath.Ext(filename)) != ".java"{
            continue
        }

		this.serializationCode[filename] = strings.Replace(serializationCode, "public final class", "final class", -1)
		this.serializationCode[filename] = strings.Replace(this.serializationCode[filename], "public static final class", "static final class", -1)

		res += this.serializationCode[filename]
	}

	return res, nil
}
//--------------------------------------------------------------------
func (this *HostCompiler) getClassPath() string{

	classPath := make([]string, 0)
	classPath = append(classPath, ".")
	classPath = append(classPath, fmt.Sprintf("%v%vprotobuf-java-3.15.2.jar", os.Getenv("METAFFI_HOME"), string(os.PathSeparator)))
	classPath = append(classPath, fmt.Sprintf("%v%vxllr.openjdk.bridge.jar", os.Getenv("METAFFI_HOME"), string(os.PathSeparator)))

	return strings.Join(classPath, string(os.PathListSeparator))
}
//--------------------------------------------------------------------
func (this *HostCompiler) buildDynamicLibrary(code string)([]byte, error){

	dir, err := os.MkdirTemp("", "metaffi_openjdk_compiler*")
	if err != nil{
		return nil, fmt.Errorf("Failed to create temp dir to build code: %v", err)
	}
	defer func(){ if err == nil{ _ = os.RemoveAll(dir) } }()

	dir = dir+string(os.PathSeparator)

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
	if err != nil{
		return nil, fmt.Errorf("Failed compiling host OpenJDK runtime linker. Exit with error: %v.\nOutput:\n%v", err, string(output))
	}

	classFiles, err := filepath.Glob(dir+"metaffi"+string(os.PathSeparator)+"*.class")
	if err != nil{
		return nil, fmt.Errorf("Failed to get list of class files to compile in the path %v*.class: %v", "metaffi", err)
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
		return nil, fmt.Errorf("Failed to read host OpenJDK runtime linker at: %v. Error: %v", this.def.IDLFilename+"_MetaFFIHost.jar", err)
	}

	return result, nil
}
//--------------------------------------------------------------------
