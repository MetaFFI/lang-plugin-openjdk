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
public final class {{$m.Name}}_Entrypoints
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

	{{range $findex, $field := $c.Fields}}
	{{if $field.Getter}}{{$retvalLength := len $field.Getter.ReturnValues}}{{$f := $field.Getter}}
    {{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
    public static void EntryPoint_{{$c.Name}}_get_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
	{
		{{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		// get object instance
		{{GetObject $c $f}}

		{{if $f.ReturnValues}}
		metaffiBridge.java_to_cdts(return_valuesCDTS, new Object[]{ {{if not $f.InstanceRequired}}{{$c.Name}}{{else}}instance{{end}}.{{$field.Name}} }, {{GetMetaFFITypes $f.ReturnValues}} );
		{{end}}
	}
	{{end}} {{/* End getter */}}

	{{if $field.Setter}}{{$f := $field.Setter}}{{$retvalLength := len $f.ReturnValues}}
    {{ReturnValuesClass $f.Name $f.ReturnValues 1}}{{$returnValuesTypeName := ReturnValuesClassName $f.Name}}
    public static void EntryPoint_{{$c.Name}}_set_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
    {
        {{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		// get object instance
		{{GetObject $c $f}}

		{{if not $f.InstanceRequired}}{{$c.Name}}{{else}}instance{{end}}.{{$field.Name}} = ({{ToJavaType $field.Type $field.Dimensions}})parameters[1];
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

	{{/* Methods */}}
	{{range $findex, $f := $c.Methods}}

	{{$retvalLength := len $f.ReturnValues}}{{$paramsLength := len $f.Parameters}}
	public static void EntryPoint_{{$c.Name}}_{{$f.Name}}(long xcall_params) throws MetaFFIException, Exception
	{
		{{GetCDTSPointers $f.Parameters $f.ReturnValues 2}}

		{{GetParamsFromCDTS $f.Parameters 2}}

		// call method
		{{GetObject $c $f}}
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
#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif
#include <string>
#include <stdexcept>
#include <functional>
#include <regex>
#include <vector>
#include <set>
#include <unordered_map>
#include <cdt_structs.h>
#include <sstream>

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <Windows.h>
#endif

{{/* Load entrypoints module function which loads all the entrypoints */}}

JavaVM* jvm = nullptr;


#define check_and_throw_jvm_exception(env, var, before_throw_code) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
env->ExceptionClear();\
before_throw_code \
throw std::runtime_error(err_msg);\
}\
else if(!var)\
{\
before_throw_code; \
throw std::runtime_error("Failed to get " #var);\
}

#define if_jvm_exception_set_error(env) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
env->ExceptionClear();\
*out_err = (char*)calloc(1, err_msg.size()+1);\
strcpy(*out_err, err_msg.c_str());\
*out_err_len = err_msg.length();\
}

struct block_guard
{
	block_guard(std::function<void()> f):f(f){}
	~block_guard()
	{
		try{ f(); } catch(...){printf("block_guard function threw an exception\n");}
	}

private:
	std::function<void()> f;
};

std::string get_exception_description(JNIEnv* penv, jthrowable throwable)
{
	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	if(!throwable_class)
	{
		return "failed to FindClass java/lang/Throwable";
	}

	jclass StringWriter_class = penv->FindClass("java/io/StringWriter");
    if(!StringWriter_class)
    {
        return "failed to FindClass java/io/StringWriter";
    }

	jclass PrintWriter_class = penv->FindClass("java/io/PrintWriter");
    if(!PrintWriter_class)
    {
        return "failed to FindClass java/io/PrintWriter";
    }

	jmethodID throwable_printStackTrace = penv->GetMethodID(throwable_class,"printStackTrace","(Ljava/io/PrintWriter;)V");
	if(!throwable_printStackTrace)
    {
        return "failed to GetMethodID throwable_printStackTrace";
    }

    jmethodID StringWriter_Constructor = penv->GetMethodID(StringWriter_class,"<init>","()V");
	if(!StringWriter_Constructor)
    {
        return "failed to GetMethodID StringWriter_Constructor";
    }

    jmethodID PrintWriter_Constructor = penv->GetMethodID(PrintWriter_class,"<init>","(Ljava/io/Writer;)V");
    if(!PrintWriter_Constructor)
    {
        return "failed to GetMethodID PrintWriter_Constructor";
    }

    jmethodID StringWriter_toString = penv->GetMethodID(StringWriter_class,"toString","()Ljava/lang/String;");
    if(!StringWriter_toString)
    {
        return "failed to GetMethodID StringWriter_toString";
    }

	// StringWriter sw = new StringWriter();
	jobject sw = penv->NewObject(StringWriter_class, StringWriter_Constructor);
	if(!sw)
    {
        return "Failed to create StringWriter object";
    }

    // PrintWriter pw = new PrintWriter(sw)
    jobject pw = penv->NewObject(PrintWriter_class, PrintWriter_Constructor, sw);
	if(!pw)
    {
        return "Failed to create PrintWriter object";
    }

    // throwable.printStackTrace(pw);
	penv->CallObjectMethod(throwable, throwable_printStackTrace, pw);
    if(!pw)
    {
        return "Failed to call printStackTrace";
    }

	// sw.toString()
	jobject str = penv->CallObjectMethod(sw, StringWriter_toString);
    if(!pw)
    {
        return "Failed to call printStackTrace";
    }

	std::string res(penv->GetStringUTFChars((jstring)str, nullptr));

	penv->DeleteLocalRef(sw);
	penv->DeleteLocalRef(pw);
	penv->DeleteLocalRef(str);

	return res;
}

typedef jclass (*load_class_t)(JNIEnv* env, const char* path, const char* class_name);
load_class_t load_class = nullptr;

std::function<void()> get_environment(JNIEnv** env)
{
	bool did_attach_thread = false;
	// Check if the current thread is attached to the VM
	auto get_env_result = jvm->GetEnv((void**)env, JNI_VERSION_10);
	if (get_env_result == JNI_EDETACHED)
	{
		if(jvm->AttachCurrentThread((void**)env, nullptr) == JNI_OK)
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

const char* get_dynamic_lib_suffix()
{
#if _WIN32
	return ".dll";
#elif __apple__
	return ".dylib";
#else
	return ".so";
#endif
}

extern "C" const char* load_entrypoints(const char* module_path, uint32_t module_path_len, JavaVM* pjvm, JNIEnv* env, load_class_t load_class)
{
	try
	{
	printf("++++ first line %s:%d\n", __FILE__, __LINE__);
		//load_class = pload_class;
	printf("++++ %s:%d\n", __FILE__, __LINE__);
		char* out_err_data = nullptr;
		printf("++++ %s:%d\n", __FILE__, __LINE__);
		uint64_t out_err_len_data = 0;
		printf("++++ %s:%d\n", __FILE__, __LINE__);
		char** out_err = &out_err_data;
		printf("++++ %s:%d\n", __FILE__, __LINE__);
		uint64_t* out_err_len = &out_err_len_data;
printf("++++ %s:%d\n", __FILE__, __LINE__);
		std::string mod_path(module_path, module_path_len);
printf("++++ %s:%d\n", __FILE__, __LINE__);
		jvm = pjvm;
printf("++++ %s:%d\n", __FILE__, __LINE__);
	    {{range $mindex, $m := .Modules}}
	        {{range $cindex, $c := $m.Classes}}

	            {{range $findex, $field := $c.Fields}}
	                {{if $field.Getter}}{{$f := $field.Getter}}
					{{if IsExternalResources $m}}
	printf("++++ 1 %s:%d\n", __FILE__, __LINE__);
	                jclass_{{$c.Name}}_get_{{$f.Name}} = load_class(env, mod_path.c_str(), "metaffi_guest.{{index $f.FunctionPath "entrypoint_class"}}");
	printf("++++ 2 %s:%d\n", __FILE__, __LINE__);
	                {{else}}
	                jclass_{{$c.Name}}_get_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
	                {{end}}

	                check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_get_{{$f.Name}},);
	                if(out_err_data){ throw std::runtime_error(out_err_data); }
	                jmethod_{{$c.Name}}_get_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_get_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
	                check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_get_{{$f.Name}},);
	                if(out_err_data){ throw std::runtime_error(out_err_data); }
	                {{end}}
	                {{if $field.Setter}}{{$f := $field.Setter}}
	                {{if IsExternalResources $m}}
	printf("++++ 3 %s:%d\n", __FILE__, __LINE__);
	                jclass_{{$c.Name}}_set_{{$f.Name}} = load_class(env, mod_path.c_str(), "metaffi_guest.{{index $f.FunctionPath "entrypoint_class"}}");
	printf("++++ 4 %s:%d\n", __FILE__, __LINE__);
	                {{else}}
	                jclass_{{$c.Name}}_set_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
	                {{end}}

	                check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_set_{{$f.Name}},);
	                if(out_err_data){ throw std::runtime_error(out_err_data); }
	                jmethod_{{$c.Name}}_set_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_set_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
	                check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_set_{{$f.Name}},);
	                if(out_err_data){ throw std::runtime_error(out_err_data); }
	                {{end}}
	            {{end}}

	            {{range $cstrindex, $f := $c.Constructors}}
		            {{if IsExternalResources $m}}
	printf("++++ 5 %s:%d\n", __FILE__, __LINE__);
	                jclass_{{$c.Name}}_{{$f.Name}} = load_class(env, mod_path.c_str(), "metaffi_guest.{{index $f.FunctionPath "entrypoint_class"}}");
	printf("++++ 6 %s:%d\n", __FILE__, __LINE__);
	                {{else}}
	                jclass_{{$c.Name}}_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
	                {{end}}

		            check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_{{$f.Name}},);
		            if(out_err_data){ throw std::runtime_error(out_err_data); }
		            jmethod_{{$c.Name}}_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
		            check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_{{$f.Name}},);
		            if(out_err_data){ throw std::runtime_error(out_err_data); }
	            {{end}}

	            {{if $c.Releaser}}{{$f := $c.Releaser}}
	            {{if IsExternalResources $m}}
	printf("++++ 7 %s:%d\n", __FILE__, __LINE__);
	            jclass_{{$c.Name}}_{{$f.Name}} = load_class(env, mod_path.c_str(), "metaffi_guest.{{index $f.FunctionPath "entrypoint_class"}}");
	printf("++++ 8 %s:%d\n", __FILE__, __LINE__);
	            {{else}}
	            jclass_{{$c.Name}}_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
	            {{end}}

	            check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_{{$f.Name}},);
	            if(out_err_data){ throw std::runtime_error(out_err_data); }
	            jmethod_{{$c.Name}}_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
	            check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_{{$f.Name}},);
	            if(out_err_data){ throw std::runtime_error(out_err_data); }
	            {{end}}

	            {{range $cstrindex, $f := $c.Methods}}
	            {{if IsExternalResources $m}}
	printf("++++ 9 %s:%d\n", __FILE__, __LINE__);
	            jclass_{{$c.Name}}_{{$f.Name}} = load_class(env, mod_path.c_str(), "metaffi_guest.{{index $f.FunctionPath "entrypoint_class"}}");
	printf("++++ 10 %s:%d\n", __FILE__, __LINE__);
	            {{else}}
	            jclass_{{$c.Name}}_{{$f.Name}} = env->FindClass("metaffi_guest/{{index $f.FunctionPath "entrypoint_class"}}");
	            {{end}}

	            check_and_throw_jvm_exception(env, jclass_{{$c.Name}}_{{$f.Name}},);
	            if(out_err_data){ throw std::runtime_error(out_err_data); }
	            jmethod_{{$c.Name}}_{{$f.Name}} = env->GetStaticMethodID(jclass_{{$c.Name}}_{{$f.Name}}, "{{index $f.FunctionPath "entrypoint_function"}}", ("(J)V"));
	            check_and_throw_jvm_exception(env, jmethod_{{$c.Name}}_{{$f.Name}},);
	            if(out_err_data){ throw std::runtime_error(out_err_data); }
	            {{end}}

	        {{end}}
	    {{end}}
    }
    catch(const std::runtime_error& err)
    {
		char* c = (char*)calloc(1, strlen(err.what())+1);
		strcpy(c, err.what());
        return c;
    }
    catch(const std::exception& err)
    {
        char* c = (char*)calloc(1, strlen(err.what())+1);
        strcpy(c, err.what());
        return c;
    }
    catch(...)
    {
        const char* m = "Failed with unknown reason";
        char* c = (char*)calloc(1, strlen(m)+1);
        strcpy(c, m);
        return c;
    }

    printf("++++ got to the end %s:%d\n", __FILE__, __LINE__);

    return nullptr;
}

{{/*
Every entrypoint calls the Java entrypoint.
The signature of the entrypoint corresponds to the expected C-function returned by load_function
*/}}


{{range $mindex, $m := .Modules}}
    {{range $cindex, $c := $m.Classes}}

        {{range $findex, $field := $c.Fields}}
            {{if $field.Getter}}{{$f := $field.Getter}}
extern "C" void EntryPoint_{{$c.Name}}_get_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_get_" $f.Name) (print "jmethod_" $c.Name "_get_" $f.Name) $f.FunctionDefinition}}
    if_jvm_exception_set_error(env);
    releaser();
}
            {{end}}
            {{if $field.Setter}}{{$f := $field.Setter}}
extern "C" void EntryPoint_{{$c.Name}}_set_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_set_" $f.Name) (print "jmethod_" $c.Name "_set_" $f.Name) $f.FunctionDefinition}}
    if_jvm_exception_set_error(env);
    releaser();
}
            {{end}}
        {{end}}

        {{range $cstrindex, $f := $c.Constructors}}
extern "C" void EntryPoint_{{$c.Name}}_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_" $f.Name) (print "jmethod_" $c.Name "_" $f.Name) $f.FunctionDefinition}}
    if_jvm_exception_set_error(env);
    releaser();
}
        {{end}}

        {{if $c.Releaser}}{{$f := $c.Releaser}}
extern "C" void EntryPoint_{{$c.Name}}_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_" $f.Name) (print "jmethod_" $c.Name "_" $f.Name) $f.FunctionDefinition}}
    if_jvm_exception_set_error(env);
    releaser();
}
        {{end}}

        {{range $cstrindex, $f := $c.Methods}}
extern "C" void EntryPoint_{{$c.Name}}_{{$f.Name}}({{CEntrypointParameters $f.FunctionDefinition}})
{
    {{CEntrypointCallJVMEntrypoint (print "jclass_" $c.Name "_" $f.Name) (print "jmethod_" $c.Name "_" $f.Name) $f.FunctionDefinition}}
    if_jvm_exception_set_error(env);
    releaser();
}
        {{end}}

    {{end}}
{{end}}
`
