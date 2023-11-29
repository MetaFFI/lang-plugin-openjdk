package main

import (
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
)

//--------------------------------------------------------------------
func javaTypeToMFFI(typename string) IDL.MetaFFIType {

	switch typename {
	case "float":
		return IDL.FLOAT32
	case "float[]":
		return IDL.FLOAT32_ARRAY
	case "double":
		return IDL.FLOAT64
	case "double[]":
		return IDL.FLOAT64_ARRAY
	case "String":
		return IDL.STRING8
	case "String[]":
		return IDL.STRING8_ARRAY
	case "byte":
		return IDL.INT8
	case "byte[]":
		return IDL.INT8_ARRAY
	case "short":
		return IDL.INT16
	case "short[]":
		return IDL.INT16_ARRAY
	case "int":
		return IDL.INT32
	case "int[]":
		return IDL.INT32_ARRAY
	case "long":
		return IDL.INT64
	case "long[]":
		return IDL.INT64_ARRAY
	case "boolean":
		return IDL.BOOL
	case "boolean[]":
		return IDL.BOOL_ARRAY

	case "Object":
		return IDL.ANY

	default:
		return IDL.HANDLE
	}

}

//--------------------------------------------------------------------
