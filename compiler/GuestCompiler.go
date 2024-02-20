package main

import (
	"fmt"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
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
func (this *GuestCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, guestOptions map[string]string) (err error) {
	return fmt.Errorf("Guest compiler is not required for OpenJDK")
}

//--------------------------------------------------------------------
