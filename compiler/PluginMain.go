package main
import (
	"github.com/OpenFFI/plugin-sdk/compiler/go"
)

import "C"

var pluginMain *LanguagePluginMain

//--------------------------------------------------------------------
type LanguagePluginMain struct{
}
//--------------------------------------------------------------------
func NewGoLanguagePluginMain() *LanguagePluginMain{
	this := &LanguagePluginMain{}
	compiler.CreateLanguagePluginInterfaceHandler(this)
	return this
}
//--------------------------------------------------------------------
func (this *LanguagePluginMain) CompileToGuest(idlDefinition *compiler.IDLDefinition, outputPath string, serializationCode map[string]string) error{

	cmp := NewCompiler(idlDefinition, serializationCode, outputPath)
	_, err := cmp.CompileGuest()
	return err
}
//--------------------------------------------------------------------
func (this *LanguagePluginMain) CompileFromHost(idlDefinition *compiler.IDLDefinition, outputPath string, serializationCode map[string]string, hostOptions map[string]string) error{

	cmp := NewCompiler(idlDefinition, serializationCode, outputPath)
	_, err := cmp.CompileHost(hostOptions)
	return err
}
//--------------------------------------------------------------------
//export init_plugin
func init_plugin(){
	pluginMain = NewGoLanguagePluginMain()
}
//--------------------------------------------------------------------
func main(){}
//--------------------------------------------------------------------
