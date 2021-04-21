#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <memory>
#include "jvm.h"
#include <utils/scope_guard.hpp>
#include <utils/function_loader.hpp>
#include <boost/filesystem.hpp>
#include <runtime/runtime_plugin_api.h>
#include <utils/function_path_parser.h>
#include <sstream>
#include <utils/foreign_function.h>
#include "classes_repository.h"
#include <map>

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

std::map<int64_t, std::pair<jclass, jmethodID>> foreign_functions;

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
int64_t load_function(const char* function_path, uint32_t function_path_len, char** err, uint32_t* err_len)
{
	openffi::utils::function_path_parser fp(std::string(function_path, function_path_len));
	
	// get module
	jclass pclass = classes_repository::get_instance().get(fp[function_path_entry_openffi_guest_lib], fp[function_path_class_entrypoint_function], true);
	
	auto* env = (JNIEnv*)(*pjvm);
	
	jmethodID methID = env->GetStaticMethodID(pclass, fp[function_path_entry_entrypoint_function].c_str(), ("([B)Lopenffi/CallResult;"));
	check_and_throw_jvm_exception(pjvm, env);
	
	int64_t function_id = foreign_functions.empty() ? 0 : -1;
	for(auto& entry : foreign_functions)
	{
		if(entry.first > function_id){
			function_id = entry.first + 1;
		}
	}
	
	foreign_functions[function_id] = std::pair(pclass, methID);
	
	return function_id;
}
//--------------------------------------------------------------------
void free_function(int64_t module_len, char** err, uint32_t* err_len)
{
}
//--------------------------------------------------------------------
void call(
		int64_t function_id,
		unsigned char* in_params, uint64_t in_params_len,
		unsigned char** out_params, uint64_t* out_params_len,
		unsigned char** out_ret, uint64_t* out_ret_len,
		uint8_t* is_error
)
{
	try
	{
		auto it = foreign_functions.find(function_id);
		if(it == foreign_functions.end())
		{
			throw std::runtime_error("given function id is not found");
		}
		
		jmethodID methID = it->second.second;
		jclass pclass = it->second.first;
		auto* env = (JNIEnv*)(*pjvm);
		
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
