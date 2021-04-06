#include "api.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <memory>
#include "jvm.h"
#include <utils/scope_guard.hpp>
#include <utils/function_loader.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include "classes_repository.h"

using namespace openffi::utils;

#define TRUE 1
#define FALSE 0

std::shared_ptr<jvm> pjvm;


#define catch_and_fill(err, err_len, ...)\
catch(std::exception& exp) \
{\
    __VA_ARGS__; \
	int len = strlen(exp.what());\
	char* errbuf = (char*)calloc(len+1, sizeof(char));\
	strcpy(errbuf, exp.what());\
	*err = errbuf;\
	*err_len = len;\
}\
catch(...)\
{\
	__VA_ARGS__;\
	int len = strlen("Unknown Error");\
	char* errbuf = (char*)calloc(len+1, sizeof(char));\
	strcpy(errbuf, "Unknown Error");\
	*err = errbuf;\
	*err_len = len;\
}

//--------------------------------------------------------------------
void load_runtime(char** err, uint32_t* err_len)
{
	try
	{
		pjvm = std::make_shared<jvm>();
		classes_repository::get_instance().init(pjvm);
	}
	catch_and_fill(err, err_len);
}
//--------------------------------------------------------------------
void free_runtime(char** err, uint32_t* err_len)
{
	try
	{
		if(pjvm)
		{
			pjvm->fini();
			pjvm = nullptr;
		}
	}
	catch_and_fill(err, err_len);
}
//--------------------------------------------------------------------
void load_module(const char* mod, uint32_t module_len, char** err, uint32_t* err_len)
{
	try
	{
		// if fails throws an exception which is handled by xllr_api.cpp.
		classes_repository::get_instance().add(std::string(mod, module_len));
	}
	catch_and_fill(err, err_len);
	
}
//--------------------------------------------------------------------
void free_module(const char* mod, uint32_t module_len, char** err, uint32_t* err_len)
{
	try
	{
		// if fails throws an exception which is handled by xllr_api.cpp.
		classes_repository::get_instance().remove(std::string(mod, module_len));
	}
	catch_and_fill(err, err_len);
}
//--------------------------------------------------------------------
void call(
		const char* mod, uint32_t module_name_len,
		const char* func_name, uint32_t func_name_len,
		unsigned char* in_params, uint64_t in_params_len,
		unsigned char** out_params, uint64_t* out_params_len,
		unsigned char** out_ret, uint64_t* out_ret_len,
		uint8_t* is_error
)
{
	try
	{
		
		// get module
		jclass pclass = classes_repository::get_instance().get(std::string(mod, module_name_len));
		
		auto* env = (JNIEnv*)(*pjvm);
		
		jmethodID methID = env->GetStaticMethodID(pclass, std::string(func_name, func_name_len).c_str(), ("([B)Lopenffi/CallResult;"));
		check_and_throw_jvm_exception(pjvm, env);
		
		jbyteArray in_params_arr = env->NewByteArray(in_params_len);
		check_and_throw_jvm_exception(pjvm, env);
		
		scope_guard sg([&]() { env->DeleteLocalRef(in_params_arr); });
		
		
		env->SetByteArrayRegion(in_params_arr, 0, in_params_len, (const jbyte *) in_params);
		check_and_throw_jvm_exception(pjvm, env);
		
		// get CallResult
		jobject result = env->CallStaticObjectMethod(pclass, methID, in_params_arr);
		check_and_throw_jvm_exception(pjvm, env);
		
		scope_guard sg_del_res([&]() { env->DeleteLocalRef(result); });
		
		jclass call_result = env->FindClass("openffi/CallResult");
		check_and_throw_jvm_exception(pjvm, env);
		
		// get from CallResult the return value or error
		jfieldID out_ret_id = env->GetFieldID(call_result, "out_ret", "[B");
		check_and_throw_jvm_exception(pjvm, env);
		
		jobject out_ret_obj = env->GetObjectField(result, out_ret_id);
		check_and_throw_jvm_exception(pjvm, env);
		
		scope_guard sg_out_params_obj([&]() { env->DeleteLocalRef(in_params_arr); });
		
		jbyte *out_ret_ptr = env->GetByteArrayElements((jbyteArray) out_ret_obj, nullptr);
		check_and_throw_jvm_exception(pjvm, env);
		
		*out_ret_len = env->GetArrayLength((jbyteArray) out_ret_obj);
		check_and_throw_jvm_exception(pjvm, env);
		
		*out_ret = (unsigned char *) calloc(*out_ret_len, sizeof(char));
		
		memcpy(*out_ret, out_ret_ptr, *out_ret_len);
	}
	catch_and_fill((char**)out_ret, out_ret_len, *is_error=TRUE);
}
//--------------------------------------------------------------------
