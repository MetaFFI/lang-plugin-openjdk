#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <memory>
#include "jvm.h"
#include <utils/scope_guard.hpp>
#include <utils/function_loader.hpp>
#include <boost/filesystem.hpp>
#include <runtime/runtime_plugin_api.h>
#include <sstream>
#include <mutex>
#include <utils/foreign_function.h>
#include <utils/function_path_parser.h>
#include <map>

using namespace openffi::utils;

#define TRUE 1
#define FALSE 0

std::shared_ptr<jvm> pjvm;
std::once_flag once_flag;


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

std::map<int64_t, std::string> foreign_functions;

//--------------------------------------------------------------------
void load_runtime(char** /*err*/, uint32_t* /*err_len*/)
{
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
	std::call_once(once_flag, [&]()->void
	{
		openffi::utils::function_path_parser fp(std::string(function_path, function_path_len));
		pjvm = std::make_shared<jvm>(fp["classpath"]);
	});
	
	
	int64_t function_id = foreign_functions.empty() ? 0 : -1;
	for(auto& entry : foreign_functions)
	{
		if(entry.first > function_id){
			function_id = entry.first + 1;
		}
	}
	
	foreign_functions[function_id] = std::string(function_path, function_path_len);
	
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
		// load function on each call, as if the call comes from different threads, the jclass and jmethodid
		// might be invalid and crash JVM
		// TODO: find a way to cache jclass and jmethodid (or make sure it is not possible)
		auto it = foreign_functions.find(function_id);
		if(it == foreign_functions.end())
		{
			throw std::runtime_error("given function id is not found");
		}
		
		const std::string& function_path = it->second;
		jclass pclass = nullptr;
		jmethodID methID = nullptr;
		pjvm->load_function_path(function_path, &pclass, &methID);
		
		JNIEnv* penv;
		auto release_env = pjvm->get_environment(&penv);
		scope_guard sg_env([&](){ release_env(); });
		
		jbyteArray in_params_arr = penv->NewByteArray(in_params_len);
		check_and_throw_jvm_exception(pjvm, penv, in_params_arr);
		
		scope_guard sg([&]() { penv->DeleteLocalRef(in_params_arr); });
		
		penv->SetByteArrayRegion(in_params_arr, 0, in_params_len, (const jbyte *) in_params);
		check_and_throw_jvm_exception(pjvm, penv, true);
		
		// get CallResult
		jobject result = penv->CallStaticObjectMethod(pclass, methID, in_params_arr);
		check_and_throw_jvm_exception(pjvm, penv, result);
		scope_guard sg_del_res([&]() { penv->DeleteLocalRef(result); });
		
		jclass call_result = penv->FindClass("openffi/CallResult");
		check_and_throw_jvm_exception(pjvm, penv, call_result);
		
		// get from CallResult the return value or error
		jfieldID out_ret_id = penv->GetFieldID(call_result, "out_ret", "[B");
		check_and_throw_jvm_exception(pjvm, penv, out_ret_id);
		
		jobject out_ret_obj = penv->GetObjectField(result, out_ret_id); // null if function is void and no error
		
		if(out_ret_obj)
		{
			scope_guard sg_out_params_obj([&]() { penv->DeleteLocalRef(in_params_arr); });
			jbyte *out_ret_ptr = penv->GetByteArrayElements((jbyteArray) out_ret_obj, nullptr);
			check_and_throw_jvm_exception(pjvm, penv, *out_ret_ptr);
		
			*out_ret_len = penv->GetArrayLength((jbyteArray) out_ret_obj);
			check_and_throw_jvm_exception(pjvm, penv, *out_ret_len);
		
			*out_ret = (unsigned char *) calloc(*out_ret_len, sizeof(char));
		
			memcpy(*out_ret, out_ret_ptr, *out_ret_len);
		}
	}
	catch_and_fill((char**)out_ret, out_ret_len, *is_error=TRUE);
}
//--------------------------------------------------------------------
