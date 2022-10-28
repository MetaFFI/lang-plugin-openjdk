package main

import (
	"fmt"
	. "github.com/MetaFFI/plugin-sdk/compiler/go/CodeTemplates"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"golang.org/x/text/cases"
	"golang.org/x/text/language"
	"strconv"
	"strings"
)

//--------------------------------------------------------------------
var templatesFuncMap = map[string]any{
	"GenerateCodeAllocateCDTS":     generateCodeAllocateCDTS,
	"SetParamsToCDTS":              setParamsToCDTS,
	"GetParamsFromCDTS":            getParamsFromCDTS,
	"XCallMetaFFI":                 xCallMetaFFI,
	"SetReturnToCDTSAndReturn":     setReturnToCDTSAndReturn,
	"ReturnValuesClass":            returnValuesClass,
	"ToJavaType":                   toJavaType,
	"Title":                        title,
	"ReturnValuesClassName":        returnValuesClassName,
	"GetMetaFFITypes":              getMetaFFITypes,
	"CallGuestMethod":              callGuestMethod,
	"ReturnGuest":                  returnGuest,
	"GetObject":                    getObject,
	"Panic":                        Panic,
	"GetCDTSPointers":              getCDTSPointers,
	"GetImports":                   getImports,
	"CEntrypointParameters":        centrypointParameters,
	"CEntrypointCallJVMEntrypoint": centrypointCallJVMEntrypoint,
	"CallConstructor":              callConstructor,
}

//--------------------------------------------------------------------
func callConstructor(class *IDL.ClassDefinition, constructor *IDL.ConstructorDefinition) string {
	
	paramsCode := ""
	if constructor.Parameters != nil {
		for i := 0; i < len(constructor.Parameters); i = i + 1 {
			paramsCode += fmt.Sprintf("(%v)parameters[%v]", toJavaType(constructor.Parameters[i].Type, constructor.Parameters[i].Dimensions), i)
			if i+1 < len(constructor.Parameters) {
				paramsCode += ", "
			}
		}
	}
	
	clsName := ""
	if pck, found := constructor.FunctionPath["package"]; found {
		clsName = pck + "." + class.Name
	} else {
		clsName = class.Name
	}
	
	code := fmt.Sprintf("var instance = new %v(%v);", clsName, paramsCode)
	
	// set object
	
	return code
}

//--------------------------------------------------------------------
func getImports(definition *IDL.IDLDefinition) string {
	
	imports := make([]string, 0)
	
	for _, m := range definition.Modules {
		for _, c := range m.Classes {
			if pck, found := c.FunctionPath["package"]; found {
				imports = append(imports, pck)
			}
		}
	}
	
	res := ""
	
	for _, imp := range imports {
		res += fmt.Sprintf("import %v.*;\n", imp)
	}
	
	return res
}

//--------------------------------------------------------------------
func Panic(msg string) string {
	panic(msg)
	return ""
}

//--------------------------------------------------------------------
func getMetaFFITypes(args []*IDL.ArgDefinition) string {
	res := "new long[]{"
	for i, p := range args {
		res += strconv.FormatUint(IDL.ArgMetaFFIType(p), 10)
		if i < len(args)-1 {
			res += ","
		}
	}
	res += "}"
	return res
}

//--------------------------------------------------------------------
func returnValuesClassName(name string) string {
	return name + "Result"
}

//--------------------------------------------------------------------
func title(elem string) string {
	caser := cases.Title(language.Und, cases.NoLower)
	elem = strings.ReplaceAll(elem, "_", " ")
	elem = caser.String(elem)
	return strings.ReplaceAll(elem, " ", "")
}

//--------------------------------------------------------------------
func getFullClassName(cls *IDL.ClassDefinition) string {
	if pck, found := cls.FunctionPath["package"]; found {
		return fmt.Sprintf("%v.%v", pck, cls.Name)
	} else {
		return cls.Name
	}
}

//--------------------------------------------------------------------
func getObject(cls *IDL.ClassDefinition, args []*IDL.ArgDefinition) string {
	
	if len(args) == 0 {
		panic("Object handle is expected in parameters[0]")
	}

	code := fmt.Sprintf("var instance = (%v)parameters[0];\n", getFullClassName(cls)) // TODO: needs to be fixed after we can see exception data

	return code
}

//--------------------------------------------------------------------
func toJavaType(elem IDL.MetaFFIType, dims int) string {
	if dims > 0 && strings.Index(string(elem), "_array") == -1 {
		elem += "_array"
	}
	javaType, found := MetaFFITypeToJavaType[elem]
	if !found {
		panic("Type \"" + elem + "\" is not a MetaFFI type")
	}
	
	return javaType
}

//--------------------------------------------------------------------
func returnValuesClass(typeName string, retval []*IDL.ArgDefinition, indent int) string {
	/*
		{{if gt $ReturnValuesLength 1}}
		public class {{$f.ReturnValuesType}}
		{
			{{{range $index, $elem := $f.ReturnValues}}public {{$elem.Type}} {{$elem.Name}};
			{{end}}
		}
		{{end}}
	*/
	
	if len(retval) < 2 {
		return ""
	}
	
	indentStr := strings.Repeat("\t", indent)
	
	res := fmt.Sprintf("public static class %vResult\n", typeName)
	res += fmt.Sprintf("%v{\n", indentStr)
	for _, rv := range retval {
		res += fmt.Sprintf("%v\tpublic %v %v;\n", indentStr, toJavaType(rv.Type, rv.Dimensions), rv.Name)
	}
	res += fmt.Sprintf("%v}\n", indentStr)
	
	return res
}

//--------------------------------------------------------------------
func setReturnToCDTSAndReturn(retval []*IDL.ArgDefinition, returnValuesType string, indent int) string {
	/*
		{{if gt $ReturnValuesLength 0}}
		{{if eq $ReturnValuesLength 1}}
		return metaffiBridge.cdts_to_java(xcall_params, {{$index}});
		{{else}}
		{{$f.ReturnValuesType}} returnValuesResult = new {{$f.ReturnValuesType}}();
		{{range $index, $elem := $f.ReturnValues}}
		returnValuesResult.{{$elem.Name}} = metaffiBridge.cdts_to_java(xcall_params, {{$index}});
		{{end}}
		return returnValuesResult;
		{{end}}
	*/
	
	if len(retval) == 0 {
		return ""
	}
	
	if len(retval) == 1 {
		return fmt.Sprintf("return (%v)metaffiBridge.cdts_to_java(return_valuesCDTS, 0)[0]", toJavaType(retval[0].Type, retval[0].Dimensions))
	}
	
	indentStr := strings.Repeat("\t", indent)
	
	res := fmt.Sprintf("%v returnValuesResult = new %v();\n", returnValuesType, returnValuesType)
	res += fmt.Sprintf("%vObject[] rets = metaffiBridge.cdts_to_java(return_valuesCDTS, %v);\n", indentStr, len(retval))
	for i, rv := range retval {
		// res += fmt.Sprintf("%vSystem.out.println(rets[%v]);\n", indentStr, i)
		res += fmt.Sprintf("%vreturnValuesResult.%v = (%v)rets[%v];\n", indentStr, rv.Name, toJavaType(rv.Type, rv.Dimensions), i)
	}
	res += fmt.Sprintf("%vreturn returnValuesResult;\n", indentStr)
	
	return res
}

//--------------------------------------------------------------------
func xCallMetaFFI(params []*IDL.ArgDefinition, retval []*IDL.ArgDefinition, funcIDName string, indent int) string {
	// metaffiBridge.{{xcall $f.Parameters $f.ReturnValues}}({{$f.GetEntityIDName}}, xcall_params);
	return fmt.Sprintf("metaffiBridge.%v(%v, xcall_params);\n", XCallFunctionName(params, retval), funcIDName)
}

//--------------------------------------------------------------------
func returnGuest(retval []*IDL.ArgDefinition, functionName string) string {
	if len(retval) == 0 {
		return ""
	}
	
	if len(retval) > 1 {
		panic(fmt.Sprintf("multiple return values for java is illegal. function %v has more than 1 return values", functionName))
	}
	
	return fmt.Sprintf("(%v)result", toJavaType(retval[0].Type, retval[0].Dimensions))
}

//--------------------------------------------------------------------

func callGuestMethod(methdef *IDL.MethodDefinition, indent int) string {
	
	// params
	paramsStr := ""
	if len(methdef.Parameters) != 0 {
		for i, p := range methdef.Parameters {
			if methdef.InstanceRequired && i == 0 { // In case calling a method (not static), skip the first parameter as it is the object
				continue
			}
			
			paramsStr += fmt.Sprintf("(%v)parameters[%v]", toJavaType(p.Type, p.Dimensions), i)
			if i < len(paramsStr)-1 {
				paramsStr += ", "
			}
		}
	}
	paramsStr = fmt.Sprintf("(%v)", paramsStr)
	
	// ret
	retStr := ""
	if len(methdef.ReturnValues) > 1 { // illegal, java function does not support more than 1
		panic(fmt.Sprintf("function %v signature is illegal. function has more than 1 return values", methdef.Name))
	}
	
	if len(methdef.ReturnValues) != 0 {
		retStr = "var result = "
	}
	
	// func name
	name := ""
	if methdef.InstanceRequired {
		name = fmt.Sprintf("%v.%v", "instance", methdef.Name)
	} else {
		name = fmt.Sprintf("%v.%v", methdef.FunctionPath["class"], methdef.Name)
	}
	
	return retStr + name + paramsStr + ";"
}

//--------------------------------------------------------------------
func getParamsFromCDTS(params []*IDL.ArgDefinition, indent int) string {
	
	/*
				{{if $f.Parameters}}
		        // get parameters from CDTS
		        Object[] parameters = cdts_to_java(xcall_params, {{MetaFFITypes $f.Parameters}});
		        {{end}}
	*/
	
	if len(params) == 0 {
		return ""
	}
	
	res := fmt.Sprintf("Object[] parameters = metaffiBridge.cdts_to_java(parametersCDTS, %v);", len(params))
	
	return res
}

//--------------------------------------------------------------------
func setParamsToCDTS(params []*IDL.ArgDefinition, indent int) string {
	/*
		{{if gt $ParametersLength 0}}
		{{range $index, $elem := $f.Parameters}}
		metaffiBridge.java_to_cdts({{$elem.Name}}, xcall_params, {{$index}});
		{{end}}
		{{end}}
	*/
	
	if len(params) == 0 {
		return ""
	}
	
	indentStr := strings.Repeat("\t", indent)
	
	metaffiTypes := "new long[]{ "
	res := "Object[] objectParams = new Object[]{ "
	for i, p := range params {
		res += p.Name
		if i < len(params)-1 {
			res += ", "
		}
		
		metaffiTypes += strconv.FormatUint(IDL.ArgMetaFFIType(p), 10)
		if i < len(params)-1 {
			metaffiTypes += ", "
		}
	}
	res += " };\n"
	metaffiTypes += " }"
	
	res += fmt.Sprintf("%vmetaffiBridge.java_to_cdts(parametersCDTS, objectParams, %v);\n", indentStr, metaffiTypes)
	
	return res
}

//--------------------------------------------------------------------
func getCDTSPointers(params []*IDL.ArgDefinition, retval []*IDL.ArgDefinition, indent int) string {
	
	indentStr := strings.Repeat("\t", indent)
	
	code := ""
	
	if len(params) > 0 {
		code += "long parametersCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)0);\n"
		
		if len(retval) > 0 {
			code += fmt.Sprintf("%vlong return_valuesCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)1);\n", indentStr)
		}
	} else {
		code += "return_valuesCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)0);\n"
	}
	
	return code
}

//--------------------------------------------------------------------
func generateCodeAllocateCDTS(params []*IDL.ArgDefinition, retval []*IDL.ArgDefinition) string {
	/*
		parametersCDTS := C.xllr_alloc_cdts_buffer( {{$paramsLength}} )
		return_valuesCDTS := C.xllr_alloc_cdts_buffer( {{$returnLength}} )
	*/
	
	if len(params) > 0 || len(retval) > 0 { // use convert_host_params_to_cdts to allocate CDTS
		code := fmt.Sprintf("long xcall_params = metaffiBridge.alloc_cdts((byte)%v, (byte)%v);\n", len(params), len(retval))
		
		if len(params) > 0 {
			code += "\t\tlong parametersCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)0);\n"
			
			if len(retval) > 0 {
				code += "\t\tlong return_valuesCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)1);\n"
			}
		} else {
			code += "\t\treturn_valuesCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)0);\n"
		}
		
		return code
		
	} else {
		return ""
	}
}

//--------------------------------------------------------------------
func centrypointParameters(meth *IDL.FunctionDefinition) string {
	
	if len(meth.Parameters) == 0 && len(meth.ReturnValues) == 0 {
		return `char** out_err, uint64_t* out_err_len`
	} else if len(meth.Parameters) > 0 && len(meth.ReturnValues) > 0 {
		return `cdts xcall_params[2], char** out_err, uint64_t* out_err_len`
	} else {
		return `cdts xcall_params[1], char** out_err, uint64_t* out_err_len`
	}
	
}

//--------------------------------------------------------------------
func centrypointCallJVMEntrypoint(jclass string, jmethod string, meth *IDL.FunctionDefinition) string {
	
	code := "JNIEnv* env;\n"
	code += "auto releaser = get_environment(&env);\n"
	
	code += fmt.Sprintf(`env->CallStaticObjectMethod(%v, %v`, jclass, jmethod)
	
	if len(meth.Parameters) == 0 && len(meth.ReturnValues) == 0 {
		code += ");\n"
	} else {
		code += ", (jlong)xcall_params);\n"
	}
	
	return code
}

//--------------------------------------------------------------------
