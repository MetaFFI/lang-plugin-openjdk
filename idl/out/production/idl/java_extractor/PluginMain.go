package main

import (
	compiler "github.com/MetaFFI/plugin-sdk/compiler/go"
)

import "C"

//--------------------------------------------------------------------
//export init_plugin
func init_plugin() {
	compiler.CreateIDLPluginInterfaceHandler(NewJavaIDLCompiler())
}

//--------------------------------------------------------------------

func main() {}
