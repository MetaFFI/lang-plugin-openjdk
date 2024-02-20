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
#include "utils/scope_guard.hpp"
#include "utils/function_path_parser.h"
#include "utils/library_loader.h"
#include "class_loader.h"
#include <utils/scope_guard.hpp>
#include "cdts_java_wrapper.h"
#include "runtime_id.h"
#include "contexts.h"


#define TRUE 1
#define FALSE 0

std::shared_ptr<jvm> pjvm;
std::once_flag once_flag;

#define handle_err(err, err_len, desc) \
	*err_len = strlen( desc ); \
	*err = (char*)malloc(*err_len + 1); \
	strcpy(*err, desc ); \
	memset((*err+*err_len), 0, 1);

#define catch_and_fill(err, err_len, ...)\
catch(std::exception& exp) \
{                                \
    __VA_ARGS__; \
	int len = strlen(exp.what());\
	char* errbuf = (char*)calloc(len+1, sizeof(char));\
	strcpy(errbuf, exp.what());\
	*err = errbuf;\
	*err_len = len;\
}\
catch(...)\
{                                        \
	__VA_ARGS__;\
	int len = strlen("Unknown Error");\
	char* errbuf = (char*)calloc(len+1, sizeof(char));\
	strcpy(errbuf, "Unknown Error");\
	*err = errbuf;\
	*err_len = len;\
}


//--------------------------------------------------------------------
void load_runtime(char** /*err*/, uint32_t* /*err_len*/)
{
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
void xcall_params_ret(void* context, cdts params_ret[2], char** out_err, uint64_t* out_err_len)
{
	try
	{
		JNIEnv *env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&]
		                               { release_environment(); });
		
		openjdk_context *ctxt = (openjdk_context *) context;
		if (ctxt->field) // if field
		{
			if (ctxt->is_getter)
			{
				if (!ctxt->instance_required)
				{
					jni_class cls(env, ctxt->cls);
					cdts_java_wrapper wrapper(params_ret[1].pcdt, params_ret[1].len);
					cls.write_field_to_cdts(0, wrapper, nullptr, ctxt->field, ctxt->field_or_return_type);
				}
				else
				{
					// get "this"
					cdts_java_wrapper params_wrapper(params_ret[0].pcdt, params_ret[0].len);
					if (params_wrapper[0]->type != metaffi_handle_type)
					{
						handle_err(out_err, out_err_len, "expecting \"this\" as first parameter");
					}
					
					jobject thisobj = (jobject) params_wrapper[0]->cdt_val.metaffi_handle_val.val;
					
					jni_class cls(env, ctxt->cls);
					cdts_java_wrapper retval_wrapper(params_ret[1].pcdt, params_ret[1].len);
					cls.write_field_to_cdts(0, retval_wrapper, thisobj, ctxt->field, ctxt->field_or_return_type);
				}
			}
			else // setter
			{
				if (!ctxt->instance_required)
				{
					cdts_java_wrapper params_wrapper(params_ret[0].pcdt, params_ret[0].len);
					
					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(0, params_wrapper, nullptr, ctxt->field);
				}
				else
				{
					cdts_java_wrapper params_wrapper(params_ret[0].pcdt, params_ret[0].len);
					jobject thisobj = (jobject) params_wrapper.get_metaffi_handle(0);
					
					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(1, params_wrapper, thisobj, ctxt->field);
				}
			}
		}
		else // callable
		{
			cdts_java_wrapper params_wrapper(params_ret[0].pcdt, params_ret[0].len);
			cdts_java_wrapper retvals_wrapper(params_ret[1].pcdt, params_ret[1].len);
			jni_class cls(env, ctxt->cls);
			cls.call(params_wrapper, retvals_wrapper, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);
		}
	}
	catch(std::runtime_error& err)
	{
		handle_err(out_err, out_err_len, err.what());
	}
	catch(...)
	{
		handle_err(out_err, out_err_len, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
void xcall_params_no_ret(void* context, cdts parameters[1], char** out_err, uint64_t* out_err_len)
{
	try
	{
		JNIEnv *env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&]{ release_environment(); });
		
		openjdk_context *ctxt = (openjdk_context *) context;
		if (ctxt->field) // if field
		{
			if (ctxt->is_getter)
			{
				throw std::runtime_error("wrong xcall for getter");
			}
			else // setter
			{
				if (!ctxt->instance_required)
				{
					cdts_java_wrapper params_wrapper(parameters[0].pcdt, parameters[0].len);
					
					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(0, params_wrapper, nullptr, ctxt->field);
				}
				else
				{
					cdts_java_wrapper params_wrapper(parameters[0].pcdt, parameters[0].len);
					
					if(params_wrapper[0]->type != metaffi_handle_type){
						throw std::runtime_error("expected \"this\" in index 0");
					}
					
					jobject thisobj = (jobject)params_wrapper[0]->cdt_val.metaffi_handle_val.val;
					
					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(1, params_wrapper, thisobj, ctxt->field);
				}
			}

		}
		else // callable
		{
			cdts_java_wrapper params_wrapper(parameters[0].pcdt, parameters[0].len);
			cdts_java_wrapper dummy(parameters[1].pcdt, parameters[1].len);
			
			jni_class cls(env, ctxt->cls);
			cls.call(params_wrapper, dummy, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);
		}
	}
	catch(std::runtime_error& err)
	{
		handle_err(out_err, out_err_len, err.what());
	}
	catch(...)
	{
		handle_err(out_err, out_err_len, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
void xcall_no_params_ret(void* context, cdts return_values[1], char** out_err, uint64_t* out_err_len)
{
	try
	{
		JNIEnv *env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&]
		                               { release_environment(); });
		
		openjdk_context *ctxt = (openjdk_context *) context;

		if (ctxt->field) // if field
		{
			if (ctxt->is_getter)
			{
				if (!ctxt->instance_required)
				{
					jni_class cls(env, ctxt->cls);
					cdts_java_wrapper wrapper(return_values[1].pcdt, return_values[1].len);
					cls.write_field_to_cdts(0, wrapper, nullptr, ctxt->field, ctxt->field_or_return_type);
				}
				else
				{
					throw std::runtime_error("wrong xcall for non-static getter");
				}
			}
			else // setter
			{
				throw std::runtime_error("wrong xcall for setter");
			}
		}
		else // callable
		{
			cdts_java_wrapper dummy(return_values[0].pcdt, return_values[0].len);
			cdts_java_wrapper retvals_wrapper(return_values[1].pcdt, return_values[1].len);
			
			jni_class cls(env, ctxt->cls);
			cls.call(dummy, retvals_wrapper, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);
		}
	}
	catch(std::runtime_error& err)
	{
		handle_err(out_err, out_err_len, err.what());
	}
	catch(...)
	{
		handle_err(out_err, out_err_len, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
void xcall_no_params_no_ret(void* context, char** out_err, uint64_t* out_err_len)
{
	try
	{
		JNIEnv *env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&]
		                               { release_environment(); });

		openjdk_context *ctxt = (openjdk_context *) context;
		if (ctxt->field) // if field
		{
			if (ctxt->is_getter)
			{
				throw std::runtime_error("wrong xcall for getter");
			}
			else // setter
			{
				throw std::runtime_error("wrong xcall for setter");
			}
		}
		else // callable
		{
			cdts_java_wrapper dummy(nullptr, 0);

			jni_class cls(env, ctxt->cls);
			cls.call(dummy, dummy, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);

		}
	}
	catch(std::runtime_error& err)
	{
		handle_err(out_err, out_err_len, err.what());
	}
	catch(...)
	{
		handle_err(out_err, out_err_len, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
void** load_function(const char* module_path, uint32_t module_path_len, const char* function_path, uint32_t function_path_len, metaffi_types_with_alias_ptr params_types, metaffi_types_with_alias_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len)
{
	void** res = nullptr;
	try
	{
		metaffi::utils::function_path_parser fp(std::string(function_path, function_path_len));
		if(!fp.contains("class"))
		{
			throw std::runtime_error("Missing class in function path");
		}

		JNIEnv* env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&]{ release_environment(); });
		
		std::string classToLoad = fp["class"];

		jni_class_loader cloader(env, std::string(module_path, module_path_len));
		
		openjdk_context* ctxt = new openjdk_context();
		
		jni_class loaded_class = cloader.load_class(classToLoad);
		ctxt->cls = (jclass)loaded_class;
		ctxt->instance_required = fp.contains("instance_required");

		void* entrypoint = nullptr;
		
		// set in context and parameters indices of "any_type"
		for(uint8_t i=0 ; i<params_count ; i++)
		{
			if(params_types[i].type == metaffi_any_type){
				ctxt->any_type_indices.insert(i);
			}
		}

		if(fp.contains("field"))
		{
			if(fp.contains("getter"))
			{
				ctxt->is_getter = true;
				
				if(retval_count == 0){
					throw std::runtime_error("return type of getter is not specified");
				}
				
				ctxt->field = loaded_class.load_field(fp["field"], argument_definition(retvals_types[0]), fp.contains("instance_required"));
				ctxt->field_or_return_type = retvals_types[0].type;
				entrypoint = fp.contains("instance_required") ? (void*)xcall_params_ret : (void*)xcall_no_params_ret;
			}
			else if(fp.contains("setter"))
			{
				ctxt->is_getter = false;
				argument_definition argtype = fp.contains("instance_required") ? argument_definition(params_types[1]) : argument_definition(params_types[0]);
				ctxt->field = loaded_class.load_field(fp["field"], argtype, fp.contains("instance_required"));
				ctxt->field_or_return_type = argtype.type;
				entrypoint = (void*)xcall_params_no_ret;
			}
			else
			{
				throw std::runtime_error("field in function path expects either getter or setter");
			}
		}
		else if(fp.contains("callable"))
		{
			if(retval_count > 1){
				throw std::runtime_error("Java does not support multiple return values");
			}

			std::vector<argument_definition> parameters;
			if(params_count > 0)
			{
				int i = ctxt->instance_required ? 1 : 0; // if instance required, skip "this"
				for(; i<params_count ; i++){
					parameters.emplace_back(params_types[i]);
				}
			}

			ctxt->method = loaded_class.load_method(fp["callable"],
													retval_count == 0 ? argument_definition() : argument_definition(retvals_types[0]),
													parameters,
													fp.contains("instance_required"));

			ctxt->constructor = fp["callable"] == "<init>";
			ctxt->field_or_return_type = retval_count == 0 ? metaffi_null_type : retvals_types[0].type;

			if (params_count == 0 && retval_count == 0)
			{
				entrypoint = (void*)xcall_no_params_no_ret;
			}
			else if (params_count > 0 && retval_count == 0)
			{
				entrypoint = (void*)xcall_params_no_ret;
			}
			else if (params_count == 0 && retval_count > 0)
			{
				entrypoint = (void*)xcall_no_params_ret;
			}
			else if (params_count > 0 && retval_count > 0)
			{
				entrypoint = (void*)xcall_params_ret;
			}
			else
			{
				entrypoint = nullptr;
			}


		}
		else
		{
			throw std::runtime_error("function path must contain either callable or field");
		}

		if(!entrypoint)
		{
			std::stringstream ss;
			ss << "Failed to detect suitable entrypoint. Params count: " << params_count << ". Retvals count: " << retval_count;
			throw std::runtime_error(ss.str().c_str());
		}

		res = (void**)malloc(sizeof(void*)*2);
		
		res[0] = (void*)entrypoint;
		res[1] = ctxt;
	}
	catch_and_fill(err, err_len);

	return res;
}
//--------------------------------------------------------------------
void** make_callable(void* make_callable_context, metaffi_types_with_alias_ptr params_types, metaffi_types_with_alias_ptr retvals_types, uint8_t params_count, uint8_t retval_count, char** err, uint32_t* err_len)
{
	void** res = nullptr;
	try
	{
		openjdk_context* ctxt = (openjdk_context*)make_callable_context;

		void* entrypoint = nullptr;

		// set in context and parameters indices of "any_type"
		for(uint8_t i=0 ; i<params_count ; i++)
		{
			if(params_types[i].type == metaffi_any_type){
				ctxt->any_type_indices.insert(i);
			}
		}

		if(retval_count > 1){
			throw std::runtime_error("Java does not support multiple return values");
		}

		std::vector<argument_definition> parameters;
		if(params_count > 0)
		{
			int i = ctxt->instance_required ? 1 : 0; // if instance required, skip "this"
			for(; i<params_count ; i++){
				parameters.emplace_back(params_types[i]);
			}
		}

		ctxt->field_or_return_type = retval_count == 0 ? metaffi_null_type : retvals_types[0].type;

		entrypoint = (params_count == 0 && retval_count == 0) ? (void*)xcall_no_params_no_ret  :
		             (params_count > 0 && retval_count == 0) ? (void*)xcall_params_no_ret :
		             (params_count == 0 && retval_count > 0) ? (void*)xcall_no_params_ret :
		             (params_count > 0 && retval_count > 0) ? (void*)xcall_params_ret :
					 nullptr;

		if(!entrypoint)
		{
			std::stringstream ss;
			ss << "Failed to detect suitable entrypoint. Params count: " << params_count << ". Retvals count: " << retval_count;
			throw std::runtime_error(ss.str().c_str());
		}

		res = (void**)malloc(sizeof(void*)*2);

		res[0] = (void*)entrypoint;
		res[1] = ctxt;
	}
	catch_and_fill(err, err_len);

	return res;
}
//--------------------------------------------------------------------
void free_function(void* pff, char** err, uint32_t* err_len)
{
}
//--------------------------------------------------------------------
