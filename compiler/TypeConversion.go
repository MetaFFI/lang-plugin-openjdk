package main

import "github.com/MetaFFI/plugin-sdk/compiler/go"

var MetaFFITypeToJavaType = map[string]string{

	compiler.FLOAT64: "double",
	compiler.FLOAT32: "float",

	compiler.INT8: "byte",
	compiler.INT16: "short",
	compiler.INT32: "int",
	compiler.INT64: "long",

	compiler.UINT8: "byte",
	compiler.UINT16: "short",
	compiler.UINT32: "int",
	compiler.UINT64: "long",

	compiler.BOOL: "boolean",

	compiler.STRING: "String",
	compiler.STRING8: "String",
	compiler.STRING16: "String",
	compiler.STRING32: "String",

	compiler.BYTES: "byte[]",
}