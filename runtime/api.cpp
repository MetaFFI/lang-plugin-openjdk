#include <cstdlib>
#include <cstring>
#include <memory>
#include <set>
#include "jvm.h"
#include <boost/algorithm/string.hpp>
#include <runtime/runtime_plugin_api.h>
#include <sstream>
#include <mutex>
#include <utils/foreign_function.h>
#include <runtime/cdt_capi_loader.h>
#include <map>
#include "cdts_java.h"
#include "utils/scope_guard.hpp"
#include "utils/function_path_parser.h"
#include "utils/library_loader.h"


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
	// TODO: find another way to change classpath
	pjvm = std::make_shared<jvm>();
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
std::shared_ptr<boost::dll::shared_library> lib; // TODO: support multiple libs!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
std::vector<std::shared_ptr<boost::dll::detail::import_type<void(cdts[2],char**,int64_t*)>::type>> params_no_params_or_no_ret_funcs;
std::vector<std::shared_ptr<boost::dll::detail::import_type<void(char**,int64_t*)>::type>> params_no_params_no_ret_funcs;

bool is_first = true;
void* load_function(const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len)
{
	void* res = nullptr;
	try
	{
		metaffi::utils::function_path_parser fp(function_path);
		
		if(is_first)// TODO: Replace with something better!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
			is_first = false;
			
			std::string dylib_to_load = fp[function_path_entry_metaffi_guest_lib];
			lib = metaffi::utils::load_library(dylib_to_load);
			
			auto load_entrypoints = lib->get<void(JavaVM*,JNIEnv*)>("load_entrypoints");
			
			JNIEnv* env;
			auto releaser = pjvm->get_environment(&env);
			
			load_entrypoints((JavaVM*)(*pjvm), env);
			releaser();
		}
		
		auto pentrypoint = lib->get<void(cdts[2],char**,int64_t*)>(fp[function_path_entry_entrypoint_function]);
		if(!pentrypoint){
			throw std::runtime_error(std::string("Failed to load: ")+fp[function_path_entry_entrypoint_function]);
		}
		
		res = (void*)pentrypoint;
		
	}
	catch_and_fill(err, err_len);
	
	
	return res;
}
//--------------------------------------------------------------------
void free_function(void* pff, char** err, uint32_t* err_len)
{
}
//--------------------------------------------------------------------
