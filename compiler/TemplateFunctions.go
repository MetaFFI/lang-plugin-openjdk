package main

import (
	"fmt"
	"regexp"
	"strconv"
	"strings"

	. "github.com/MetaFFI/sdk/compiler/go/CodeTemplates"
	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
	"golang.org/x/text/cases"
	"golang.org/x/text/language"
)

// --------------------------------------------------------------------
var templatesFuncMap = map[string]any{
	"GenerateCodeAllocateCDTS":     generateCodeAllocateCDTS,
	"SetParamsToCDTS":              setParamsToCDTS,
	"GetParamsFromCDTS":            getParamsFromCDTS,
	"XCallMetaFFI":                 xCallMetaFFI,
	"SetReturnToCDTSAndReturn":     setReturnToCDTSAndReturn,
	"ReturnValuesClass":            returnValuesClass,
	"ToJavaType":                   ToJavaType,
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
	"ExternalResourcesAsArray":     externalResourcesAsArray,
	"IsExternalResources":          isExternalResources,
	"CreateLoadFunction":           createLoadFunction,
	"GetFullClassName":             getFullClassName,
	"ClassForName":                 classForName,
	// Modern MetaFFI API functions
	"ConvertToJavaType":  convertToJavaType,
	"GetMetaFFIType":     getMetaFFIType,
	"IsArray":            isArray,
	"toJavaType":         toJavaType,
	"getMetaFFITypeInfo": getMetaFFITypeInfo,
}

func classForName(className string) string {
	return strings.ReplaceAll(className, "$", "_")
}

// --------------------------------------------------------------------
func isExternalResources(module *IDL.ModuleDefinition) bool {
	return len(module.ExternalResources) > 0
}

// --------------------------------------------------------------------
func externalResourcesAsArray(module *IDL.ModuleDefinition) string {
	res := make([]string, 0)

	for _, r := range module.ExternalResources {
		res = append(res, fmt.Sprintf(`R"(%v)"`, r))
	}

	if len(res) > 0 {
		return "," + strings.Join(res, ",")
	} else {
		return ""
	}

}

// --------------------------------------------------------------------
func callConstructor(class *IDL.ClassDefinition, constructor *IDL.ConstructorDefinition) string {

	paramsCode := ""
	if constructor.Parameters != nil {
		for i := 0; i < len(constructor.Parameters); i = i + 1 {
			paramsCode += fmt.Sprintf("(%v)parameters[%v]", ToJavaType(constructor.Parameters[i]), i)
			if i+1 < len(constructor.Parameters) {
				paramsCode += ", "
			}
		}
	}

	clsName := ""
	if pck, found := constructor.EntityPath["package"]; found {
		clsName = pck + "." + class.Name
	} else {
		clsName = class.Name
	}

	clsName = strings.ReplaceAll(clsName, "$", ".")

	code := fmt.Sprintf("var instance = new %v(%v);", clsName, paramsCode)

	// set object

	return code
}

// --------------------------------------------------------------------
func getImports(definition *IDL.IDLDefinition) string {

	imports := make([]string, 0)

	for _, m := range definition.Modules {
		for _, c := range m.Classes {
			if pck, found := c.EntityPath["package"]; found {
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

// --------------------------------------------------------------------
func Panic(msg string) string {
	panic(msg)
	return ""
}

// --------------------------------------------------------------------
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

// --------------------------------------------------------------------
func returnValuesClassName(name string) string {
	return name + "Result"
}

// --------------------------------------------------------------------
func title(elem string) string {
	caser := cases.Title(language.Und, cases.NoLower)
	elem = strings.ReplaceAll(elem, "_", " ")
	elem = caser.String(elem)
	return strings.ReplaceAll(elem, " ", "")
}

// --------------------------------------------------------------------
func getFullClassName(cls *IDL.ClassDefinition) string {

	re := regexp.MustCompile(`\$[0-9]+`)
	clsName := re.ReplaceAllString(cls.Name, "")
	clsName = strings.ReplaceAll(clsName, "$", ".")

	if pck, found := cls.EntityPath["package"]; found {
		return fmt.Sprintf("%v.%v", pck, clsName)
	} else {
		return clsName
	}
}

// --------------------------------------------------------------------
func getObject(cls *IDL.ClassDefinition, meth *IDL.MethodDefinition) string {

	if !meth.InstanceRequired {
		return ""
	}

	args := meth.Parameters

	if len(args) == 0 {
		panic("Object handle is expected in parameters[0]")
	}

	code := fmt.Sprintf("var instance = (%v)parameters[0];\n", getFullClassName(cls)) // TODO: needs to be fixed after we can see exception data

	return code
}

// --------------------------------------------------------------------
func ToJavaType(arg *IDL.ArgDefinition) string {

	elem := string(arg.Type)
	dims := arg.Dimensions

	if dims > 0 && strings.Index(elem, "_array") == -1 {
		elem += "_array"
	}
	javaType, found := MetaFFITypeToJavaType[elem]
	if !found {
		panic("Type \"" + elem + "\" is not a MetaFFI type")
	}

	if strings.HasSuffix(javaType, "Object") && arg.IsTypeAlias() {
		alias := strings.ReplaceAll(arg.TypeAlias, "$", ".")
		javaType = strings.ReplaceAll(javaType, "Object", alias)
	}

	return javaType
}

// --------------------------------------------------------------------
func returnValuesClass(typeName string, retval []*IDL.ArgDefinition, indent int, cls *IDL.ClassDefinition) string {
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
		res += fmt.Sprintf("%v\tpublic %v %v;\n", indentStr, ToJavaType(rv), rv.Name)
	}
	res += fmt.Sprintf("%v}\n", indentStr)

	return res
}

// --------------------------------------------------------------------
func setReturnToCDTSAndReturn(retval []*IDL.ArgDefinition, returnValuesType string, indent int, cls *IDL.ClassDefinition) string {
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
		return fmt.Sprintf("return (%v)metaffiBridge.cdts_to_java(return_valuesCDTS, 1)[0];", ToJavaType(retval[0]))
	}

	indentStr := strings.Repeat("\t", indent)

	res := fmt.Sprintf("%v returnValuesResult = new %v();\n", returnValuesType, returnValuesType)
	res += fmt.Sprintf("%vObject[] rets = metaffiBridge.cdts_to_java(return_valuesCDTS, %v);\n", indentStr, len(retval))
	for i, rv := range retval {
		res += fmt.Sprintf("%vreturnValuesResult.%v = (%v)rets[%v];\n", indentStr, rv.Name, ToJavaType(rv), i)
	}
	res += fmt.Sprintf("%vreturn returnValuesResult;\n", indentStr)

	return res
}

// --------------------------------------------------------------------
func xCallMetaFFI(params []*IDL.ArgDefinition, retval []*IDL.ArgDefinition, funcIDName string, indent int) string {
	// metaffiBridge.{{xcall $f.Parameters $f.ReturnValues}}({{$f.GetEntityIDName}}, xcall_params);
	res := fmt.Sprintf("metaffiBridge.%v(%v", XCallFunctionName(params, retval), funcIDName)
	if len(params) == 0 && len(retval) == 0 {
		res += ");\n"
	} else {
		res += ", xcall_params);\n"
	}

	return res
}

// --------------------------------------------------------------------
func returnGuest(retval []*IDL.ArgDefinition, functionName string, cls *IDL.ClassDefinition) string {
	if len(retval) == 0 {
		return ""
	}

	if len(retval) > 1 {
		panic(fmt.Sprintf("multiple return values for java is illegal. function %v has more than 1 return values", functionName))
	}

	return fmt.Sprintf("(%v)result", ToJavaType(retval[0]))
}

//--------------------------------------------------------------------

func callGuestMethod(methdef *IDL.MethodDefinition, indent int, cls *IDL.ClassDefinition) string {

	// params
	paramsStrList := make([]string, 0)
	if len(methdef.Parameters) != 0 {
		for i, p := range methdef.Parameters {
			if methdef.InstanceRequired && i == 0 { // In case calling a method (not static), skip the first parameter as it is the object
				continue
			}

			paramsStrList = append(paramsStrList, fmt.Sprintf("(%v)parameters[%v]", ToJavaType(p), i))

		}
	}

	paramsStr := strings.Join(paramsStrList, ",")
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
		name = fmt.Sprintf("%v.%v", getFullClassName(methdef.GetParent()), methdef.Name)
	}

	return retStr + name + paramsStr + ";"
}

// --------------------------------------------------------------------
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

// --------------------------------------------------------------------
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

		if p.Type != IDL.ANY {
			metaffiTypes += strconv.FormatUint(IDL.ArgMetaFFIType(p), 10)
		} else {
			metaffiTypes += fmt.Sprintf("metaffiBridge.getMetaFFIType(%v)", p.Name)
		}

		if i < len(params)-1 {
			metaffiTypes += ", "
		}
	}
	res += " };\n"
	metaffiTypes += " }"

	res += fmt.Sprintf("%vmetaffiBridge.java_to_cdts(parametersCDTS, objectParams, %v);\n", indentStr, metaffiTypes)

	return res
}

// --------------------------------------------------------------------
func getCDTSPointers(params []*IDL.ArgDefinition, retval []*IDL.ArgDefinition, indent int) string {

	indentStr := strings.Repeat("\t", indent)

	code := ""

	if len(params) > 0 {
		code += "long parametersCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)0);\n"

		if len(retval) > 0 {
			code += fmt.Sprintf("%vlong return_valuesCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)1);\n", indentStr)
		}
	} else if len(retval) > 0 {
		code += "long return_valuesCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)0);\n"
	}

	return code
}

// --------------------------------------------------------------------
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
			code += "\t\tlong return_valuesCDTS = metaffiBridge.get_pcdt(xcall_params, (byte)0);\n"
		}

		return code

	} else {
		return ""
	}
}

// --------------------------------------------------------------------
func centrypointParameters(meth *IDL.FunctionDefinition) string {

	if len(meth.Parameters) == 0 && len(meth.ReturnValues) == 0 {
		return `char** out_err, uint64_t* out_err_len`
	} else if len(meth.Parameters) > 0 && len(meth.ReturnValues) > 0 {
		return `cdts xcall_params[2], char** out_err, uint64_t* out_err_len`
	} else {
		return `cdts xcall_params[1], char** out_err, uint64_t* out_err_len`
	}

}

// --------------------------------------------------------------------
func centrypointCallJVMEntrypoint(jclass string, jmethod string, meth *IDL.FunctionDefinition) string {

	code := "JNIEnv* env;\n"
	code += "auto releaser = get_environment(&env);\n"
	//	code += fmt.Sprintf(`printf("++++ before CallStaticObjectMethod(%v, %v). `, jclass, jmethod)
	//	code += `env: %p\n", env);`+"\n"
	code += fmt.Sprintf(`env->CallStaticObjectMethod(%v, %v`, jclass, jmethod)

	if len(meth.Parameters) == 0 && len(meth.ReturnValues) == 0 {
		code += ");\n"
	} else {
		code += ", (jlong)xcall_params);\n"
	}

	return code
}

// --------------------------------------------------------------------
func createLoadFunction(idl *IDL.IDLDefinition, mod *IDL.ModuleDefinition) string {
	// a JVM function is limited to 2^16 bytes.
	// therefore, every 3000 functions, split a function, and make sure the previous function calls the next one.

	loadingsPerLoadMethods := 3000

	loadFunctionLinesOfCode := make([]string, 0)
	// make lines of code to load function

	/*
		{{range $findex, $f := $m.Globals}}
		{{if $f.Getter}}{{$f.Getter.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Getter.EntityPathAsString $idl}}", (byte){{len $f.Getter.Parameters}}, (byte){{len $f.Getter.ReturnValues}});{{end}}
		{{if $f.Setter}}{{$f.Setter.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Setter.EntityPathAsString $idl}}", (byte){{len $f.Setter.Parameters}}, (byte){{len $f.Setter.ReturnValues}});{{end}}
		{{end}}{{/* End globals * /}}
	*/
	for _, f := range mod.Globals {
		if f.Getter != nil {
			line := fmt.Sprintf("%v = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", f.Getter.GetEntityIDName(), idl.TargetLanguage, f.Getter.EntityPathAsString(idl), len(f.Getter.Parameters), len(f.Getter.ReturnValues))
			loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
		}

		if f.Setter != nil {
			line := fmt.Sprintf("%v = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", f.Setter.GetEntityIDName(), idl.TargetLanguage, f.Setter.EntityPathAsString(idl), len(f.Setter.Parameters), len(f.Setter.ReturnValues))
			loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
		}
	}

	/*
		{{range $findex, $f := $m.Functions}}
		{{$f.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.EntityPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}
	*/
	for _, f := range mod.Functions {
		line := fmt.Sprintf("%v = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", f.GetEntityIDName(), idl.TargetLanguage, f.EntityPathAsString(idl), len(f.Parameters), len(f.ReturnValues))
		loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
	}

	/*
		{{range $cindex, $c := $m.Classes}}
	*/
	for _, c := range mod.Classes {

		/*
			{{range $findex, $f := $c.Fields}}
			{{if $f.Getter}}{{$c.Name}}_overload{{$f.Getter.GetNameWithOverloadIndex}}ID = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Getter.EntityPathAsString $idl}}", (byte){{len $f.Getter.Parameters}}, (byte){{len $f.Getter.ReturnValues}});{{end}}
			{{if $f.Setter}}{{$c.Name}}_overload{{$f.Setter.GetNameWithOverloadIndex}}ID = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Setter.EntityPathAsString $idl}}", (byte){{len $f.Setter.Parameters}}, (byte){{len $f.Setter.ReturnValues}});{{end}}
			{{end}}
		*/
		for _, f := range c.Fields {
			if f.Getter != nil {
				line := fmt.Sprintf("%v_%vID = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", c.Name, f.Getter.GetNameWithOverloadIndex(), idl.TargetLanguage, f.Getter.EntityPathAsString(idl), len(f.Getter.Parameters), len(f.Getter.ReturnValues))
				loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
			}
			if f.Setter != nil {
				line := fmt.Sprintf("%v_%vID = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", c.Name, f.Setter.GetNameWithOverloadIndex(), idl.TargetLanguage, f.Setter.EntityPathAsString(idl), len(f.Setter.Parameters), len(f.Setter.ReturnValues))
				loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
			}
		}

		/*
			{{range $findex, $f := $c.Constructors}}
			{{$c.Name}}_{{$f.GetNameWithOverloadIndex}}ID = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.EntityPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
			{{end}}
		*/
		for _, f := range c.Constructors {
			line := fmt.Sprintf("%v_%vID = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", c.Name, f.GetNameWithOverloadIndex(), idl.TargetLanguage, f.EntityPathAsString(idl), len(f.Parameters), len(f.ReturnValues))
			loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
		}

		/*
			{{if $c.Releaser}}{{$f := $c.Releaser}}
			{{$f.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.EntityPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
			{{end}}
		*/
		if f := c.Releaser; f != nil {
			line := fmt.Sprintf("%v_%vID = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", c.Name, f.GetNameWithOverloadIndex(), idl.TargetLanguage, f.EntityPathAsString(idl), len(f.Parameters), len(f.ReturnValues))
			loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
		}

		/*
			{{range $findex, $f := $c.Methods}}
			{{$f.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.EntityPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
			{{end}}
		*/
		for _, f := range c.Methods {
			line := fmt.Sprintf("%v_%vID = metaffiBridge.load_function(\"xllr.%v\", modulePath, \"%v\", (byte)%v, (byte)%v);", c.Name, f.GetNameWithOverloadIndex(), idl.TargetLanguage, f.EntityPathAsString(idl), len(f.Parameters), len(f.ReturnValues))
			loadFunctionLinesOfCode = append(loadFunctionLinesOfCode, line)
		}
	}

	// place them inside "load" functions

	res := "public static void metaffi_load(String modulePath)\n"
	res += "\t{\n"
	res += fmt.Sprintf("\t\tmetaffiBridge.load_runtime_plugin(\"xllr.%v\");\n", idl.TargetLanguage) // metaffiBridge.load_runtime_plugin("xllr.{{$targetLanguage}}");
	res += "\t\t\n"

	for i, line := range loadFunctionLinesOfCode {

		if i > 0 && i%loadingsPerLoadMethods == 0 { // 3000 "loadings" - start a new method
			res += fmt.Sprintf("\t\tload%v(modulePath);\n", i+1) // call next load function
			res += "\t}\n"                                       // close current load method
			res += "\t\n"
			res += fmt.Sprintf("\tpublic static void load%v(String modulePath)\n", i+1) // create next load method
			res += "\t{\n"
		}

		res += fmt.Sprintf("\t\t%v\n", line)
	}

	res += "\t}\n"

	return res
}

//--------------------------------------------------------------------

// Modern MetaFFI API functions

// --------------------------------------------------------------------
func isArray(dimensions int) bool {
	return dimensions > 0
}

// --------------------------------------------------------------------
func convertToJavaType(arg *IDL.ArgDefinition) string {
	if arg.Dimensions > 0 {
		return convertToJavaType(arg) + "[]"
	}

	switch arg.Type {
	case "int64":
		return "long"
	case "int32":
		return "int"
	case "int16":
		return "short"
	case "int8":
		return "byte"
	case "uint64":
		return "long"
	case "uint32":
		return "int"
	case "uint16":
		return "short"
	case "uint8":
		return "byte"
	case "float64":
		return "double"
	case "float32":
		return "float"
	case "string8":
		return "String"
	case "string16":
		return "String"
	case "string32":
		return "String"
	case "bool":
		return "boolean"
	case "handle":
		if arg.TypeAlias != "" {
			return arg.TypeAlias
		}
		return "MetaFFIHandle"
	case "any":
		return "Object"
	default:
		return "Object"
	}
}

// --------------------------------------------------------------------
func getMetaFFIType(arg *IDL.ArgDefinition) string {
	switch arg.Type {
	case "int64":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)"
	case "int32":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt32)"
	case "int16":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt16)"
	case "int8":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt8)"
	case "uint64":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt64)"
	case "uint32":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt32)"
	case "uint16":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt16)"
	case "uint8":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt8)"
	case "float64":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)"
	case "float32":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat32)"
	case "string8":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)"
	case "string16":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString16)"
	case "string32":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString32)"
	case "bool":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)"
	case "handle":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)"
	case "any":
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)"
	default:
		return "new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)"
	}
}

// --------------------------------------------------------------------
// New template functions for modern MetaFFI API

// toJavaType converts MetaFFI type to Java type
func toJavaType(metaffiType string) string {
	switch metaffiType {
	case "int32":
		return "int"
	case "int64":
		return "long"
	case "float32":
		return "float"
	case "float64":
		return "double"
	case "bool":
		return "boolean"
	case "string":
		return "String"
	case "array":
		return "Object[]"
	case "map":
		return "Map"
	case "function":
		return "Object"
	case "void":
		return "void"
	case "handle":
		return "Object"
	default:
		return "Object"
	}
}

// getMetaFFITypeInfo generates MetaFFI type info for complex types
func getMetaFFITypeInfo(metaffiType string) string {
	switch metaffiType {
	case "int32":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIInt32"
	case "int64":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64"
	case "float32":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat32"
	case "float64":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64"
	case "bool":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIBool"
	case "string":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIString8"
	case "array":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIArray"
	case "map":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIAny"
	case "function":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFICallable"
	case "void":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFINull"
	case "handle":
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle"
	default:
		return "MetaFFITypeInfo.MetaFFITypes.MetaFFIAny"
	}
}

// javaKeywords contains Java reserved keywords
var javaKeywords = map[string]bool{
	"abstract":     true,
	"assert":       true,
	"boolean":      true,
	"break":        true,
	"byte":         true,
	"case":         true,
	"catch":        true,
	"char":         true,
	"class":        true,
	"const":        true,
	"continue":     true,
	"default":      true,
	"do":           true,
	"double":       true,
	"else":         true,
	"enum":         true,
	"extends":      true,
	"final":        true,
	"finally":      true,
	"float":        true,
	"for":          true,
	"goto":         true,
	"if":           true,
	"implements":   true,
	"import":       true,
	"instanceof":   true,
	"int":          true,
	"interface":    true,
	"long":         true,
	"native":       true,
	"new":          true,
	"package":      true,
	"private":      true,
	"protected":    true,
	"public":       true,
	"return":       true,
	"short":        true,
	"static":       true,
	"strictfp":     true,
	"super":        true,
	"switch":       true,
	"synchronized": true,
	"this":         true,
	"throw":        true,
	"throws":       true,
	"transient":    true,
	"try":          true,
	"void":         true,
	"volatile":     true,
	"while":        true,
}

// MetaFFITypeToJavaType maps MetaFFI types to Java types
var MetaFFITypeToJavaType = map[string]string{
	"int32":    "int",
	"int64":    "long",
	"float32":  "float",
	"float64":  "double",
	"bool":     "boolean",
	"string":   "String",
	"array":    "Object[]",
	"map":      "Map",
	"function": "Object",
	"void":     "void",
	"handle":   "Object",
	"any":      "Object",
}
