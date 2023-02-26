package main

const HostFunctionStubsTemplate = `
{{$targetLanguage := .TargetLanguage}}
{{ $idl := . }}
{{range $mindex, $m := .Modules}}
// Code to call foreign functions in module {{$m.Name}} via MetaFFI

public final class {{$m.Name}}
{
	public static metaffi.MetaFFIBridge metaffiBridge = new MetaFFIBridge();

	{{range $findex, $f := $m.Globals}}
	{{if $f.Getter}}public static long {{$f.Getter.GetEntityIDName}} = 0;{{end}}
    {{if $f.Setter}}public static long {{$f.Setter.GetEntityIDName}} = 0;{{end}}
    {{end}}{{/* End globals */}}

	{{range $findex, $f := $m.Functions}}
	public static long {{$f.GetEntityIDName}} = 0;
	{{end}}

	{{range $cindex, $c := $m.Classes}}

	{{range $findex, $f := $c.Fields}}
	{{if $f.Getter}}public static long {{$c.Name}}_{{$f.Getter.Name}}ID = 0;{{end}}
    {{if $f.Setter}}public static long {{$c.Name}}_{{$f.Setter.Name}}ID = 0;{{end}}
	{{end}}

	{{range $findex, $f := $c.Constructors}}
	public static long {{$c.Name}}_{{$f.Name}}ID = 0;
	{{end}}

	{{if $c.Releaser}}{{$f := $c.Releaser}}
	public static long {{$c.Name}}_{{$f.Name}}ID = 0;
	{{end}}

	{{range $findex, $f := $c.Methods}}
	public static long {{$c.Name}}_{{$f.Name}}ID = 0;
	{{end}}

	{{end}}{{/*End classes*/}}

	public static void load(String modulePath)
	{
		metaffiBridge.load_runtime_plugin("xllr.{{$targetLanguage}}");

		{{range $findex, $f := $m.Globals}}
		{{if $f.Getter}}{{$f.Getter.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Getter.FunctionPathAsString $idl}}", (byte){{len $f.Getter.Parameters}}, (byte){{len $f.Getter.ReturnValues}});{{end}}
	    {{if $f.Setter}}{{$f.Setter.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Setter.FunctionPathAsString $idl}}", (byte){{len $f.Setter.Parameters}}, (byte){{len $f.Setter.ReturnValues}});{{end}}
	    {{end}}{{/* End globals */}}

		{{range $findex, $f := $m.Functions}}
		{{$f.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.FunctionPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{range $cindex, $c := $m.Classes}}

		{{range $findex, $f := $c.Fields}}
		{{if $f.Getter}}{{$c.Name}}_{{$f.Getter.Name}}ID = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Getter.FunctionPathAsString $idl}}", (byte){{len $f.Getter.Parameters}}, (byte){{len $f.Getter.ReturnValues}});{{end}}
	    {{if $f.Setter}}{{$c.Name}}_{{$f.Setter.Name}}ID = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.Setter.FunctionPathAsString $idl}}", (byte){{len $f.Setter.Parameters}}, (byte){{len $f.Setter.ReturnValues}});{{end}}
		{{end}}

		{{range $findex, $f := $c.Constructors}}
		{{$c.Name}}_{{$f.Name}}ID = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.FunctionPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{if $c.Releaser}}{{$f := $c.Releaser}}
		{{$f.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.FunctionPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{range $findex, $f := $c.Methods}}
		{{$f.GetEntityIDName}} = metaffiBridge.load_function("xllr.{{$targetLanguage}}", modulePath, "{{$f.FunctionPathAsString $idl}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{end}}{{/*End classes*/}}
	}

	public static void free()
	{
		metaffiBridge.free_runtime_plugin("xllr.{{$targetLanguage}}");
	}

	{{/* --- Globals --- */}}
	{{range $findex, $f := $m.Globals}}
	{{if $f.Getter}}{{$f := $f.Getter}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	public static {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}} {{if $index}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{XCallMetaFFI $f.Parameters $f.ReturnValues $f.GetEntityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
		
	}
	{{end}}
	{{if $f.Setter}}{{$f := $f.Setter}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	public static {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}} {{if $index}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{XCallMetaFFI $f.Parameters $f.ReturnValues $f.GetEntityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
		
	}
	{{end}}
	{{end}}{{/* End globals */}}

	{{/* --- Functions --- */}}
	{{range $findex, $f := $m.Functions}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	public static {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}} {{if $index}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{XCallMetaFFI $f.Parameters $f.ReturnValues $f.GetEntityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
		
	}
	{{end}}
}
{{end}}
`

const HostClassesStubsTemplate = `
{{$c := .C}}
{{$m := .M}}
public class {{$c.Name}}
{
	private metaffi.MetaFFIHandle this_instance;
	private static metaffi.MetaFFIBridge metaffiBridge = new metaffi.MetaFFIBridge();

	{{/* --- Fields --- */}}
	{{range $findex, $fi := $c.Fields}}
	{{if $fi.Getter}}{{$f := $fi.Getter}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	public {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}}{{if gt $index 0}} {{if gt $index 1}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := (print $m.Name "." $f.GetEntityIDName)}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
		
	}
	{{end}}{{/*end getter*/}}
	{{if $fi.Setter}}{{$f := $fi.Setter}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	public {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}}{{if gt $index 0}} {{if gt $index 1}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := (print $m.Name "." $f.GetEntityIDName)}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
	}
	{{end}}{{/*end setter*/}}
	{{end}}{{/*end fields*/}}

	{{range $findex, $f := $c.Constructors}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	public {{$c.Name}}({{range $index, $elem := $f.Parameters}} {{if $index}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}} ) throws MetaFFIException
	{
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := (print $m.Name "." $c.Name "_" $f.Name "ID")}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}

		this.this_instance = (metaffi.MetaFFIHandle)metaffiBridge.cdts_to_java(return_valuesCDTS, 1)[0];
	}
	{{end}}{{/*end constructors*/}}

	{{if $c.Releaser}}{{$f := $c.Releaser}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	@Override
	public void finalize()
	{
		// TODO: Delete reference to the object in the objects table
	}
	{{end}}{{/*end releaser*/}}


	{{range $findex, $f := $c.Methods}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	public {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}}{{if gt $index 0}} {{if gt $index 1}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := (print $m.Name "." $f.GetEntityIDName)}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
	}
	{{end}}{{/*end methods*/}}
}
`

const HostHeaderTemplate = `
{{DoNotEditText "//"}}
`

const HostPackage = `package metaffi_host;
`

const HostImports = `
import java.io.*;
import java.util.*;
import metaffi.*;
`
