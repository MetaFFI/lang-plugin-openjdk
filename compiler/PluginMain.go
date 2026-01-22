package main

import (
	"github.com/MetaFFI/sdk/compiler/go/plugin"
)

import "C"

//export init_plugin
func init_plugin() {
	plugin.PluginMain = plugin.NewLanguagePluginMain(NewModernHostCompiler(), nil)
}

// --------------------------------------------------------------------
func main() {}

//--------------------------------------------------------------------
