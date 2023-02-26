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
#include "class_loader.h"


#define TRUE 1
#define FALSE 0

std::shared_ptr<jvm> pjvm;
std::once_flag once_flag;


#define catch_and_fill(err, err_len, ...)\
catch(std::exception& exp) \
{\
    printf("++++ catch exception\n");                                     \
    __VA_ARGS__; \
	int len = strlen(exp.what());\
	char* errbuf = (char*)calloc(len+1, sizeof(char));\
	strcpy(errbuf, exp.what());\
	*err = errbuf;\
	*err_len = len;\
}\
catch(...)\
{                                        \
	printf("++++ catch ...\n");        \
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
std::unordered_map<std::string, std::shared_ptr<boost::dll::shared_library>> libs;
std::vector<std::shared_ptr<boost::dll::detail::import_type<void(cdts[2],char**,int64_t*)>::type>> params_no_params_or_no_ret_funcs;
std::vector<std::shared_ptr<boost::dll::detail::import_type<void(char**,int64_t*)>::type>> params_no_params_no_ret_funcs;

std::vector<std::string> parse_module_path(const std::string& module_path)
{
	std::string tmp;
	std::stringstream ss(module_path);
	std::vector<std::string> classpath;
	
	while(std::getline(ss, tmp, ';')){
		classpath.push_back(tmp);
	}
	
	return classpath;
}
//--------------------------------------------------------------------
void* load_function(const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len)
{
	void* res = nullptr;
	try
	{printf("++++ %s:%d\n", __FILE__, __LINE__);
		metaffi::utils::function_path_parser fp(std::string(function_path, function_path_len));
		printf("++++ %s:%d\n", __FILE__, __LINE__);
		std::string modpath(module_path, module_path_len);
		auto it_lib = libs.find(modpath);
		std::shared_ptr<boost::dll::shared_library> lib;
		printf("++++ %s:%d\n", __FILE__, __LINE__);
		if(it_lib == libs.end())// TODO: make thread safe !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{printf("++++ %s:%d\n", __FILE__, __LINE__);
			std::vector<std::string> modules = parse_module_path(modpath);
			
			// first module must be the dynamic library
			lib = metaffi::utils::load_library(modules[0], false, false);
			
			printf("++++ %s:%d\n", __FILE__, __LINE__);
			auto load_entrypoints = lib->get<const char*(const char*, uint32_t, JavaVM*,JNIEnv*, load_class_t)>("load_entrypoints");
			
			JNIEnv* env;
			auto releaser = pjvm->get_environment(&env);
			printf("++++ %s:%d\n", __FILE__, __LINE__);
			const char* ep_err = load_entrypoints(module_path, module_path_len, (JavaVM*)(*pjvm), env, &load_class);
			if(ep_err)
			{
				printf("++++ %s:%d\n", __FILE__, __LINE__);
				std::string load_entrypoints_error = ep_err;
				free((void*)ep_err);
				throw std::runtime_error(load_entrypoints_error);
			}
			printf("++++ %s:%d\n", __FILE__, __LINE__);
			releaser();
			
			libs[fp[function_path_entry_metaffi_guest_lib]] = lib;
			printf("++++ %s:%d\n", __FILE__, __LINE__);
		}
		else
		{
			printf("++++ %s:%d\n", __FILE__, __LINE__);
			lib = it_lib->second;
		}
		printf("++++ %s:%d\n", __FILE__, __LINE__);
		auto pentrypoint = lib->get<void(cdts[2],char**,int64_t*)>(fp[function_path_entry_entrypoint_function]);
		if(!pentrypoint){
			throw std::runtime_error(std::string("Failed to load: ")+fp[function_path_entry_entrypoint_function]);
		}
		printf("++++ %s:%d\n", __FILE__, __LINE__);
		res = (void*)pentrypoint;
		
	}
	catch_and_fill(err, err_len);
	
	printf("+++ return load function\n");
	
	return res;
}
//--------------------------------------------------------------------
void free_function(void* pff, char** err, uint32_t* err_len)
{
}
//--------------------------------------------------------------------
