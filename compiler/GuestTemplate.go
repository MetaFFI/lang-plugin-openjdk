package main

const GuestHeaderTemplate = `
// Code generated by MetaFFI.
// Guest code for {{.IDLFilenameWithExtension}}
`

const GuestPackage = `package metaffi_guest;
`

const GuestImportsTemplate = `
import java.io.*;
import java.util.*;
import metaffi.*;
{{GetImports .}}
`

const GuestFunctionXLLRTemplate = `
{{$idlFilename := .IDLFilename}}
{{range $mindex, $m := .Modules}}


// Code to call foreign functions in module {{$m.Name}}
public final class {{$m.Name}}
{
	private static MetaFFIBridge metaffiBridge = new MetaFFIBridge();

	// Globals
	{{if $m.Globals}}
	{{Panic "OpenJDK does not support global variables"}}
	{{end}}
 
	{{if $m.Functions}}
	{{Panic "OpenJDK does not support global functions"}}
	{{end}}
	
	// classes entities
	{{range $cindex, $c := $m.Classes}}

	{{range $findex, $f := $c.Fields}}
	{{if $f.Getter}}{{$retvalLength := len $f.Getter.ReturnValues}}{{$f := $f.Getter}}
    {{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
    public static void EntryPoint_{{$c.Name}}_get_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
	{
		{{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		// get object instance
		{{GetObject $c $f.Parameters}}

		System.out.printf("Getter returning instance.{{$f.Name}}. Type: {{GetMetaFFITypes $f.ReturnValues}}\n");

		{{if $f.ReturnValues}}
		metaffiBridge.java_to_cdts(return_valuesCDTS, new Object[]{ instance.{{$f.Name}} }, {{GetMetaFFITypes $f.ReturnValues}} );
		{{end}}
	}
	{{end}} {{/* End getter */}}

	{{if $f.Setter}}{{$f := $f.Setter}}{{$retvalLength := len $f.Setter.ReturnValues}}
    {{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
    public static void EntryPoint_{{$c.Name}}_set_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
    {
        {{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		// get object instance
		{{GetObject $c $f.Parameters}}

		instance.{{$f.Name}} = parameters[1];
    }
    {{end}} {{/* End setter */}}
    {{end}} {{/*End Fields*/}}

	{{range $cstrindex, $f := $c.Constructors}}
	public static void EntryPoint_{{$c.Name}}_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
	{
		{{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		// get object instance
		{{CallConstructor $c $f}}

		{{if $f.ReturnValues}}
        metaffiBridge.java_to_cdts(return_valuesCDTS, new Object[]{ instance }, {{GetMetaFFITypes $f.ReturnValues}} );
        {{end}}
	}
	{{end}}

	{{if $c.Releaser}}{{$f := $c.Releaser}}
	public static void EntryPoint_{{$c.Name}}_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
	{
		{{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		metaffiBridge.remove_object((Long)parameters[0]);
	}
    {{end}} {{/*End Releaser*/}}

	{{range $findex, $f := $c.Methods}}

	{{$retvalLength := len $f.ReturnValues}}{{$paramsLength := len $f.Parameters}}
	public static void EntryPoint_{{$c.Name}}_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
	{
		{{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		// call method
		{{GetObject $c $f.Parameters}}
		{{ CallGuestMethod $f 2 }}

		{{if $f.ReturnValues}}
        metaffiBridge.java_to_cdts(return_valuesCDTS, new Object[]{ result }, {{GetMetaFFITypes $f.ReturnValues}} );
        {{end}}
	}
	{{end}} {{/*End Methods*/}}
	{{end}} {{/*End Classes*/}}

}
{{end}}
`

const GuestCPPEntrypoint = `
#include <jni.h>
#include <string>
#include <stdexcept>
#include <cdt_structs.h>
#include <functional>
{{/* Load entrypoints module function which loads all the entrypoints */}}

JavaVM* jvm = nullptr;


#define check_and_throw_jvm_exception(env, var, before_throw_code) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
before_throw_code \
throw std::runtime_error(err_msg);\
}\
else if(!var)\
{\
before_throw_code; \
throw std::runtime_error("Failed to get " #var);\
}

#define if_exception_throw_jvm_exception(env, before_throw_code) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
before_throw_code; \
throw std::runtime_error(err_msg);\
}

std::string get_exception_description(JNIEnv* penv, jthrowable throwable)
{
	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	if(!throwable_class)
	{
		throw std::runtime_error("failed to FindClass java/lang/Throwable");
	}

	jmethodID throwable_toString = penv->GetMethodID(throwable_class,"toString","()Ljava/lang/String;");
	if(!throwable_class)
    {
        throw std::runtime_error("failed to GetMethodID ()Ljava/lang/String;");
    }

	jobject str = penv->CallObjectMethod(throwable, throwable_toString);
	if(!throwable_class)
    {
        throw std::runtime_error("failed to CallObjectMethod ()Ljava/lang/String;");
    }

	std::string res(penv->GetStringUTFChars((jstring)str, nullptr));

	penv->DeleteLocalRef(str);

	return res;
}

std::function<void()> get_environment(JNIEnv** env)
{
	bool did_attach_thread = false;
	// Check if the current thread is attached to the VM
	auto get_env_result = jvm->GetEnv((void**)env, JNI_VERSION_10);
	if (get_env_result == JNI_EDETACHED)
	{
		if(jvm->AttachCurrentThread((void**)*env, nullptr) == JNI_OK)
		{
			did_attach_thread = true;
		}
		else
		{
			// Failed to attach thread. Throw an exception if you want to.
			throw std::runtime_error("Failed to attach environment to current thread");
		}
	}
	else if (get_env_result == JNI_EVERSION)
	{
		// Unsupported JNI version. Throw an exception if you want to.
		throw std::runtime_error("Failed to get JVM environment - unsupported JNI version");
	}

	return did_attach_thread ? std::function<void()>([](){ jvm->DetachCurrentThread(); }) : [](){};
}


{{range $mindex, $m := .Modules}}
    {{range $cindex, $c := $m.Classes}}

        {{range $findex, $f := $c.Fields}}
            {{if $f.Getter}}{{$f := $f.Getter}}
jclass jclass_{{$c.Name}}_get_{{$f.Name}} = nullptr;
jmethodID jmethod_{{$c.Name}}_get_{{$f.Name}} = nullptr;
            {{end}}
            {{if $f.Setter}}{{$f := $f.Setter}}
jclass jclass_{{$c.Name}}_set_{{$f.Name}} = nullptr;
jmethodID jmethod_{{$c.Name}}_set_{{$f.Name}} = nullptr;
            {{end}}
        {{end}}

        {{range $cstrindex, $f := $c.Constructors}}
jclass jclass_{{$c.Name}}_{{$f.Name}} = nullptr;
jmethodID jmethod_{{$c.Name}}_{{$f.Name}} = nullptr;
        {{end}}

        {{if $c.Releaser}}{{$f := $c.Releaser}}
jclass jclass_{{$c.Name}}_{{$f.Name}} = nullptr;
jmethodID jmethod_{{$c.Name}}_{{$f.Name}} = nullptr;
        {{end}}

        {{range $cstrindex, $f := $c.Methods}}
jclass jclass_{{$c.Name}}_{{$f.Name}} = nullptr;
jmethodID jmethod_{{$c.Name}}_{{$f.Name}} = nullptr;
        {{end}}

	{{end}}
{{end}}

extern "C" void load_entrypoints(JavaVM* pjvm, JNIEnv* env)
{
	jvm = pjvm;

    {{range $mindex, $m := .Modules}}
        {{range $cindex, $c := $m.Classes}}

            {{range $findex, $f := $c.Fields}}
                {{if $f.Getter}}{{$f := $f.Getter}}
                jclass_{{$c.Name}}_get_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
                check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_get_{{$f.Name}},);
                jmethod_{{$c.Name}}_get_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_get_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
                check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_get_{{$f.Name}},);
                {{end}}
                {{if $f.Setter}}{{$f := $f.Setter}}
                jclass_{{$c.Name}}_set_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
                check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_set_{{$f.Name}},);
                jmethod_{{$c.Name}}_set_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_set_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
                check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_set_{{$f.Name}},);
                {{end}}
            {{end}}

            {{range $cstrindex, $f := $c.Constructors}}
	            jclass_{{$c.Name}}_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
	            check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_{{$f.Name}},);
	            jmethod_{{$c.Name}}_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
	            check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_{{$f.Name}},);
            {{end}}

            {{if $c.Releaser}}{{$f := $c.Releaser}}
            jclass_{{$c.Name}}_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
            check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_{{$f.Name}},);
            jmethod_{{$c.Name}}_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
            check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_{{$f.Name}},);
            {{end}}

            {{range $cstrindex, $f := $c.Methods}}
            jclass_{{$c.Name}}_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
            check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_{{$f.Name}},);
            jmethod_{{$c.Name}}_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
            check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_{{$f.Name}},);
            {{end}}

        {{end}}
    {{end}}
}

{{/*
Every entrypoint calls the Java entrypoint.
The signature of the entrypoint corresponds to the expected C-function returned by load_function
*/}}


{{range $mindex, $m := .Modules}}
    {{range $cindex, $c := $m.Classes}}

        {{range $findex, $f := $c.Fields}}
            {{if $f.Getter}}{{$f := $f.Getter}}
extern "C" void EntryPoint_{{$c.Name}}_get_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_get_" $f.Name) (print "jmethod_" $c.Name "_get_" $f.Name) $f.FunctionDefinition}}
    if_exception_throw_jvm_exception(env, releaser());
    releaser();
}
            {{end}}
            {{if $f.Setter}}{{$f := $f.Setter}}
extern "C" void EntryPoint_{{$c.Name}}_set_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_set_" $f.Name) (print "jmethod_" $c.Name "_set_" $f.Name) $f.FunctionDefinition}}
    if_exception_throw_jvm_exception(env, releaser());
    releaser();
}
            {{end}}
        {{end}}

        {{range $cstrindex, $f := $c.Constructors}}
extern "C" void EntryPoint_{{$c.Name}}_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_" $f.Name) (print "jmethod_" $c.Name "_" $f.Name) $f.FunctionDefinition}}
    if_exception_throw_jvm_exception(env, releaser());
    releaser();
}
        {{end}}

        {{if $c.Releaser}}{{$f := $c.Releaser}}
extern "C" void EntryPoint_{{$c.Name}}_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_" $f.Name) (print "jmethod_" $c.Name "_" $f.Name) $f.FunctionDefinition}}
    if_exception_throw_jvm_exception(env, releaser());
    releaser();
}
        {{end}}

        {{range $cstrindex, $f := $c.Methods}}
extern "C" void EntryPoint_{{$c.Name}}_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_" $f.Name) (print "jmethod_" $c.Name "_" $f.Name) $f.FunctionDefinition}}
    if_exception_throw_jvm_exception(env, releaser());
    releaser();
}
        {{end}}

    {{end}}
{{end}}
`
