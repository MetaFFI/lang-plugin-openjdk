package main

import (
	"fmt"
	JavaExtractor "github.com/MetaFFI/lang-plugin-openjdk/idl/javaextractor"
	"github.com/MetaFFI/plugin-sdk/compiler/go"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"os"
	"path"
	"strings"
)

// --------------------------------------------------------------------
type JavaIDLCompiler struct {
	sourceCode         string
	sourceCodeFilePath string

	idl *IDL.IDLDefinition
}

// --------------------------------------------------------------------
func NewJavaIDLCompiler() *JavaIDLCompiler {

	p := os.Getenv("METAFFI_HOME")
	ext := compiler.GetDynamicLibSuffix()

	JavaExtractor.MetaFFILoad(fmt.Sprintf("%v/JavaExtractor_MetaFFIGuest%v;%v/JavaExtractor.jar;%v/asm-9.6.jar;%v/asm-tree-9.6.jar;%v/javaparser-core-3.24.4.jar;%v/JavaExtractor_MetaFFIGuest.jar", p, ext, p, p, p, p, p))
	return &JavaIDLCompiler{}
}

// --------------------------------------------------------------------
func (this *JavaIDLCompiler) ParseIDL(sourceCode string, filePath string) (*IDL.IDLDefinition, bool, error) {

	javafile, err := JavaExtractor.NewJavaExtractor(filePath)
	if err != nil {
		return nil, true, err
	}

	this.sourceCode = sourceCode
	this.sourceCodeFilePath = filePath

	javainfo, err := javafile.Extract()
	if err != nil {
		return nil, true, err
	}

	classes, err := ExtractClasses(&javainfo)
	if err != nil {
		return nil, true, err
	}

	// parse AST and build IDLDefinition

	this.idl = IDL.NewIDLDefinition(this.sourceCodeFilePath, "openjdk")

	module := IDL.NewModuleDefinition(this.idl.IDLFilename)

	for _, c := range classes {
		module.AddClass(c)
	}

	this.idl.AddModule(module)

	guestCodeModule := strings.ReplaceAll(path.Base(this.sourceCodeFilePath), path.Ext(this.sourceCodeFilePath), "")
	module.AddExternalResource(guestCodeModule + path.Ext(this.sourceCodeFilePath))

	module.SetFunctionPath("module", guestCodeModule)

	this.idl.FinalizeConstruction()

	return this.idl, true, nil
}

//--------------------------------------------------------------------