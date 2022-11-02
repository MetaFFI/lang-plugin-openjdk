package main

import "github.com/MetaFFI/plugin-sdk/compiler/go/IDL"

var MetaFFITypeToJavaType = map[IDL.MetaFFIType]string{
	
	IDL.FLOAT64:       "double",
	IDL.FLOAT64_ARRAY: "double[]",
	IDL.FLOAT32:       "float",
	IDL.FLOAT32_ARRAY: "float[]",
	
	IDL.INT8:        "byte",
	IDL.INT8_ARRAY:  "byte[]",
	IDL.INT16:       "short",
	IDL.INT16_ARRAY: "short[]",
	IDL.INT32:       "int",
	IDL.INT32_ARRAY: "int[]",
	IDL.INT64:       "long",
	IDL.INT64_ARRAY: "long[]",
	
	IDL.UINT8:        "byte",
	IDL.UINT8_ARRAY:  "byte[]",
	IDL.UINT16:       "short",
	IDL.UINT16_ARRAY: "short[]",
	IDL.UINT32:       "int",
	IDL.UINT32_ARRAY: "int[]",
	IDL.UINT64:       "long",
	IDL.UINT64_ARRAY: "long[]",
	
	IDL.BOOL:       "boolean",
	IDL.BOOL_ARRAY: "boolean[]",
	
	IDL.STRING8:        "String",
	IDL.STRING8_ARRAY:  "String[]",
	IDL.STRING16:       "String",
	IDL.STRING16_ARRAY: "String[]",
	IDL.STRING32:       "String",
	IDL.STRING32_ARRAY: "String[]",
	
	IDL.HANDLE:       "Object",
	IDL.HANDLE_ARRAY: "Object[]",
	
	IDL.ANY: "Object",
}
