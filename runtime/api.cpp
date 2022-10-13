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
#include "../../metaffi-core/XLLR/xcall_jit.h"
#include "utils/scope_guard.hpp"


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
void* load_function(const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, char** err, uint32_t* err_len)
{
	// TODO: Fix load_function to work correctly
	
	void* res = nullptr;
	
	try
	{
		jclass cls;
		jmethodID meth;
		pjvm->load_function_path(std::string(function_path, function_path_len), &cls, &meth);
		printf("+++ ---> load function %d,%d\n", params_count, retval_count);
		if(params_count > 0 && retval_count > 0)
		{
			res = (void*)(pforeign_function_entrypoint_signature_params_ret_t)create_xcall_params_ret({sizeof(cls), sizeof(meth)}, {(int64_t) cls, (int64_t) meth}, function_path,
	                   (void*) (void (*)(cdts[2], char**, uint64_t*, jclass, jmethodID)) ([](cdts pcdts[2], char** out_err, uint64_t* out_err_len, jclass cls, jmethodID meth)
	                   {
						   try
						   {printf("+++++ CALLED FUNC\n");
							   JNIEnv* env;
							   auto releaser = pjvm->get_environment(&env);
							   metaffi::utils::scope_guard env_guard([&](){ releaser(); env = nullptr; });
							   
							   // convert CDTS to Java
							   jobjectArray params = cdts_java(pcdts[0].pcdt, pcdts[0].len, env).parse();

							   // call method
							   jobject result = pjvm->call_function(meth, cls, params);
							
							   cdts_java cj(pcdts[1].pcdt, pcdts[1].len, env);
							   std::vector<metaffi_type_t> metaffi_types = cj.get_types((jobjectArray)result);
							   
							   // convert Java to CDTS
							   cj.build((jobjectArray)result, metaffi_types.data(), pcdts[0].len, 0);
						   }
						   catch_and_fill(out_err, out_err_len);
	                   }));
		}
		else if(params_count > 0)
		{
//			res = (void*)(pforeign_function_entrypoint_signature_params_no_ret_t)create_xcall_params_no_ret({sizeof(cls), sizeof(meth)}, {(int64_t) cls, (int64_t) meth}, function_path,
//                       (void*) (void (*)(cdts[2], char**, uint64_t*, jclass, jmethodID)) ([](cdts pcdts[1], char** out_err, uint64_t* out_err_len, jclass cls, jmethodID meth)
//                       {
//						   try
//						   {
//							   // convert CDTS to Java
//							   jobjectArray params = cdts_java(pcdts[0].pcdt, pcdts[0].len, pjvm).parse();
//
//							   // call method
//							   pjvm->call_function(meth, cls, params);
//
//							   // TODO: check if there's a JVM exception
//						   }
//	                       catch_and_fill(out_err, out_err_len);
//
//                       }));
		}
		else if(retval_count > 0)
		{
//			res = (void*)(pforeign_function_entrypoint_signature_no_params_ret_t)create_xcall_no_params_ret({sizeof(cls), sizeof(meth)}, {(int64_t) cls, (int64_t) meth}, function_path,
//                     (void*) (void (*)(cdts[2], char**, uint64_t*, jclass, jmethodID)) ([](cdts pcdts[1], char** out_err, uint64_t* out_err_len, jclass cls, jmethodID meth)
//                     {
//                         try
//                         {
//                             // call method
//                             jobjectArray result = (jobjectArray)pjvm->call_function(meth, cls);
//
//	                         // convert Java to CDTS
//	                         cdts_java(pcdts[1].pcdt, pcdts[1].len, pjvm).build(result, 0);
//                         }
//                         catch_and_fill(out_err, out_err_len);
//
//                     }));
		}
		else
		{
//			res = (void*)(pforeign_function_entrypoint_signature_no_params_no_ret_t)create_xcall_no_params_no_ret({sizeof(cls), sizeof(meth)}, {(int64_t) cls, (int64_t) meth}, function_path,
//                     (void*) (void (*)(char**, uint64_t*, jclass, jmethodID)) ([](char** out_err, uint64_t* out_err_len, jclass cls, jmethodID meth)
//                     {
//                         try
//                         {
//                             // call method
//                             jobjectArray result = (jobjectArray)pjvm->call_function(meth, cls);
//
//
//                         }
//                         catch_and_fill(out_err, out_err_len);
//
//                     }));
		}
		
	}
	catch_and_fill(err, err_len);
	printf("+++ <--- load function\n");
	return res;
}
//--------------------------------------------------------------------
void free_function(void* pff, char** err, uint32_t* err_len)
{
}
//--------------------------------------------------------------------
