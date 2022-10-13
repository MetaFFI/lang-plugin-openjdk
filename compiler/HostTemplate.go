package main

const HostFunctionStubsTemplate = `
{{$targetLanguage := .TargetLanguage}}
{{range $mindex, $m := .Modules}}
// Code to call foreign functions in module {{$m.Name}} via MetaFFI

public final class {{$m.Name}}
{
	public static MetaFFIBridge metaffi;

	{{range $findex, $f := $m.Globals}}
	{{if $f.Getter}}{{$f = $f.Getter}}public static long {{$f.GetEntityIDName}} = 0;{{end}}
    {{if $f.Setter}}{{$f = $f.Setter}}public static long {{$f.GetEntityIDName}} = 0;{{end}}
    {{end}}{{/* End globals */}}

	{{range $findex, $f := $m.Functions}}
	public static long {{$f.GetEntityIDName}} = 0;
	{{end}}

	{{range $cindex, $c := $m.Classes}}

	{{range $findex, $f := $c.Fields}}
	{{if $f.Getter}}{{$f = $f.Getter}}public static long {{$f.GetEntityIDName}} = 0;{{end}}
    {{if $f.Setter}}{{$f = $f.Setter}}public static long {{$f.GetEntityIDName}} = 0;;{{end}}
	{{end}}

	{{range $findex, $f := $c.Constructor}}
	public static long {{$f.GetEntityIDName}} = 0;
	{{end}}

	{{if $c.Releaser}}{{$f := $c.Releaser}}
	public static long {{$f.GetEntityIDName}} = 0;
	{{end}}

	{{range $findex, $f := $c.Methods}}
	public static long {{$f.GetEntityIDName}} = 0;
	{{end}}

	{{end}}{{/*End classes*/}}

	static
	{
		metaffi = new MetaFFIBridge();
		metaffi.load_runtime_plugin("xllr.{{$targetLanguage}}");

		{{range $findex, $f := $m.Globals}}
		{{if $f.Getter}}{{$f = $f.Getter}}{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Getter.Parameters}}, (byte){{len $f.Getter.ReturnValues}});{{end}}
	    {{if $f.Setter}}{{$f = $f.Setter}}{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Setter.Parameters}}, (byte){{len $f.Setter.ReturnValues}});{{end}}
	    {{end}}{{/* End globals */}}

		{{range $findex, $f := $m.Functions}}
		{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{range $cindex, $c := $m.Classes}}

		{{range $findex, $f := $c.Fields}}
		{{if $f.Getter}}{{$f = $f.Getter}}{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});{{end}}
	    {{if $f.Setter}}{{$f = $f.Setter}}{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});{{end}}
		{{end}}

		{{range $findex, $f := $c.Constructor}}
		{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{if $c.Releaser}}{{$f := $c.Releaser}}
		{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{range $findex, $f := $c.Methods}}
		{{$f.GetEntityIDName}} = metaffi.load_function("xllr.{{$targetLanguage}}", "{{$f.FunctionPathAsString}}", (byte){{len $f.Parameters}}, (byte){{len $f.ReturnValues}});
		{{end}}

		{{end}}{{/*End classes*/}}
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

{{/* --- Classes --- */}}
{{range $findex, $c := $m.Classes}}
public class {{$c.Name}}
{
	private long objectID;

	{{/* --- Fields --- */}}
	{{range $findex, $f := $c.Fields}}
	{{if $f.Getter}}{{$f := $f.Getter}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	public {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}} {{if $index}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := $m.Name "." $f.GetEntityIDName}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
		
	}
	{{end}}{{/*end getter*/}}
	{{if $f.Setter}}{{$f := $f.Setter}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	public {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}} {{if $index}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := $m.Name "." $f.GetEntityIDName}}
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

		{{ $entityIDName := $m.Name "." $f.GetEntityIDName}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}

		this.objectID = metaffi.CDTSToJava(xcall_params, 0)
	}
	{{end}}{{/*end constructors*/}}

	{{if $c.Releaser}}{{$f := $c.Releaser}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	@Override
	public void finalize()
	{
	    {{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := $m.Name "." $f.GetEntityIDName}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}
	}
	{{end}}{{/*end releaser*/}}


	{{range $findex, $f := $c.Methods}}
	{{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
	{{$ParametersLength := len $f.Parameters}}{{$ReturnValuesLength := len $f.ReturnValues}}
	public {{if eq $ReturnValuesLength 0}}void{{else if gt $ReturnValuesLength 1}}{{$returnValuesTypeName}}{{else}}{{$elem := index $f.ReturnValues 0}}{{ToJavaType $elem.Type $elem.Dimensions}}{{end}} {{$f.Name}}({{range $index, $elem := $f.Parameters}} {{if $index}},{{end}}{{ToJavaType $elem.Type $elem.Dimensions}} {{$elem.Name}}{{end}} ) throws MetaFFIException
	{
		{{/* creates xcall_params, parametersCDTS and return_valuesCDTS */}}
		{{GenerateCodeAllocateCDTS $f.Parameters $f.ReturnValues}}

		{{SetParamsToCDTS $f.Parameters 2}}

		{{ $entityIDName := $m.Name "." $f.GetEntityIDName}}
		{{XCallMetaFFI $f.Parameters $f.ReturnValues $entityIDName 2}}

		{{SetReturnToCDTSAndReturn $f.ReturnValues $returnValuesTypeName 2}}
	}
	{{end}}{{/*end methods*/}}
}
{{end}}{{/*end classes*/}}

{{end}}{{/*end modules*/}}
`

const HostHeaderTemplate = `
{{DoNotEditText "//"}}
`

const HostPackage = `package metaffi;
`

const HostImports = `
import java.io.*;
import java.util.*;
`
