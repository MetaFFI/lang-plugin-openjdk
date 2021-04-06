package main

import (
	"github.com/OpenFFI/plugin-sdk/compiler/go"
)

//--------------------------------------------------------------------
type Compiler struct{
	def *compiler.IDLDefinition
	serializationCode map[string]string
	outputPath string
}
//--------------------------------------------------------------------
func NewCompiler(def *compiler.IDLDefinition, serializationCode map[string]string, outputPath string) *Compiler {
	return &Compiler{def: def, serializationCode: serializationCode, outputPath: outputPath}
}
//--------------------------------------------------------------------
func (this *Compiler) CompileGuest() (outputFileName string, err error){

	cmp := NewGuestCompiler(this.def, this.outputPath, this.def.IDLFilename, this.serializationCode)
	outputFileName, err = cmp.Compile()
	if err != nil{
		return
	}

	return outputFileName, err
}
//--------------------------------------------------------------------
func (this *Compiler) CompileHost(hostOptions map[string]string) (outputFileName string, err error){

	cmp := NewHostCompiler(this.def, this.outputPath, this.def.IDLFilename, this.serializationCode, hostOptions)
	return cmp.Compile()
}
//--------------------------------------------------------------------