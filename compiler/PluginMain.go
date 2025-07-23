package main

import (
	compiler "github.com/MetaFFI/plugin-sdk/compiler/go"
)

import "C"

//export init_plugin
func init_plugin() {
	compiler.PluginMain = compiler.NewLanguagePluginMain(NewModernHostCompiler(), nil)
}

// --------------------------------------------------------------------
func main() {}

//--------------------------------------------------------------------
