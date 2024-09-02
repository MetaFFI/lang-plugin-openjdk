#include "cdts_java_wrapper.h"
#include "class_loader.h"
#include "contexts.h"
#include "jvm.h"
#include "utils/entity_path_parser.h"
#include "utils/scope_guard.hpp"
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <runtime/runtime_plugin_api.h>
#include <runtime/xllr_capi_loader.h>
#include <set>
#include <sstream>


#define TRUE 1
#define FALSE 0

std::shared_ptr<jvm> pjvm;
std::once_flag once_flag;

#define handle_err(err, desc)               \
	{                                       \
		auto err_len = std::strlen(desc);   \
		*err = (char*) malloc(err_len + 1); \
		if (!*err)                          \
		{                                   \
			throw std::runtime_error("out of memory"); \
		}                                   \
		std::copy(desc, desc + err_len, *err); \
		(*err)[err_len] = '\0';             \
	}

#define catch_and_fill(err, ...)                              \
	catch(std::exception & exp)                               \
	{                                                         \
		__VA_ARGS__;                                          \
		auto len = std::strlen(exp.what());                   \
		*err = (char*) malloc(len + 1);                       \
		if (!*err)                                            \
		{                                                     \
			throw std::runtime_error("out of memory");        \
		}                                                     \
		std::copy(exp.what(), exp.what() + len, *err);        \
		(*err)[len] = '\0';                                   \
	}                                                         \
	catch(...)                                                \
	{                                                         \
		__VA_ARGS__;                                          \
		const char* unknown_err = "Unknown Error";            \
		auto len = std::strlen(unknown_err);                  \
		*err = (char*) malloc(len + 1);                       \
		if (!*err)                                            \
		{                                                     \
			throw std::runtime_error("out of memory");        \
		}                                                     \
		std::copy(unknown_err, unknown_err + len, *err);      \
		(*err)[len] = '\0';                                   \
	}

//--------------------------------------------------------------------
void load_runtime(char** err)
{
	try
	{
		pjvm = std::make_shared<jvm>();
	}
	catch_and_fill(err);
}
//--------------------------------------------------------------------
void free_runtime(char** err)
{
	try
	{
		if(pjvm)
		{
			pjvm->fini();
			pjvm = nullptr;
		}
	}
	catch_and_fill(err);
}
//--------------------------------------------------------------------
void jni_xcall_params_ret(void* context, cdts params_ret[2], char** out_err)
{
	try
	{
		JNIEnv* env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&] { release_environment(); });

		openjdk_context* ctxt = (openjdk_context*) context;
		if(ctxt->field)// if field
		{
			if(ctxt->is_getter)
			{
				if(!ctxt->instance_required)
				{
					jni_class cls(env, ctxt->cls);
					cdts_java_wrapper wrapper(&params_ret[1]);
					cls.write_field_to_cdts(0, wrapper, nullptr, ctxt->field, ctxt->field_or_return_type);
				}
				else
				{
					// get "this"
					cdts_java_wrapper params_wrapper(&params_ret[0]);
					if(params_wrapper[0].type != metaffi_handle_type)
					{
						handle_err(out_err, "expecting \"this\" as first parameter");
					}

					jobject thisobj = (jobject) params_wrapper[0].cdt_val.handle_val->handle;

					jni_class cls(env, ctxt->cls);
					cdts_java_wrapper retval_wrapper(&params_ret[1]);
					cls.write_field_to_cdts(0, retval_wrapper, thisobj, ctxt->field, ctxt->field_or_return_type);
				}
			}
			else// setter
			{
				if(!ctxt->instance_required)
				{
					cdts_java_wrapper params_wrapper(&params_ret[0]);

					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(0, params_wrapper, nullptr, ctxt->field);
				}
				else
				{
					cdts_java_wrapper params_wrapper(&params_ret[0]);
					jobject thisobj = (jobject) params_wrapper[0].cdt_val.handle_val->handle;

					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(1, params_wrapper, thisobj, ctxt->field);
				}
			}
		}
		else// callable
		{
			cdts_java_wrapper params_wrapper(&params_ret[0]);
			cdts_java_wrapper retvals_wrapper(&params_ret[1]);
			jni_class cls(env, ctxt->cls);
			cls.call(params_wrapper, retvals_wrapper, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);
		}
	} catch(std::runtime_error& err)
	{
		handle_err(out_err, err.what());
	} catch(...)
	{
		handle_err(out_err, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
void jni_xcall_params_no_ret(void* context, cdts parameters[1], char** out_err)
{
	try
	{
		JNIEnv* env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&] { release_environment(); });

		openjdk_context* ctxt = (openjdk_context*) context;
		if(ctxt->field)// if field
		{
			if(ctxt->is_getter)
			{
				throw std::runtime_error("wrong xcall for getter");
			}
			else// setter
			{
				if(!ctxt->instance_required)
				{
					cdts_java_wrapper params_wrapper(&parameters[0]);

					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(0, params_wrapper, nullptr, ctxt->field);
				}
				else
				{
					cdts_java_wrapper params_wrapper(&parameters[0]);

					if(params_wrapper[0].type != metaffi_handle_type)
					{
						throw std::runtime_error("expected \"this\" in index 0");
					}

					jobject thisobj = (jobject) params_wrapper[0].cdt_val.handle_val->handle;

					jni_class cls(env, ctxt->cls);
					cls.write_cdts_to_field(1, params_wrapper, thisobj, ctxt->field);
				}
			}
		}
		else// callable
		{
			cdts_java_wrapper params_wrapper(&parameters[0]);
			cdts_java_wrapper dummy(&parameters[1]);

			jni_class cls(env, ctxt->cls);
			cls.call(params_wrapper, dummy, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);
		}
	} catch(std::runtime_error& err)
	{
		handle_err(out_err, err.what());
	} catch(...)
	{
		handle_err(out_err, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
void jni_xcall_no_params_ret(void* context, cdts return_values[1], char** out_err)
{
	try
	{
		JNIEnv* env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&] { release_environment(); });

		openjdk_context* ctxt = (openjdk_context*) context;

		if(ctxt->field)// if field
		{
			if(ctxt->is_getter)
			{
				if(!ctxt->instance_required)
				{
					jni_class cls(env, ctxt->cls);
					cdts_java_wrapper wrapper(&return_values[1]);
					cls.write_field_to_cdts(0, wrapper, nullptr, ctxt->field, ctxt->field_or_return_type);
				}
				else
				{
					throw std::runtime_error("wrong xcall for non-static getter");
				}
			}
			else// setter
			{
				throw std::runtime_error("wrong xcall for setter");
			}
		}
		else// callable
		{
			cdts_java_wrapper dummy(&return_values[0]);
			cdts_java_wrapper retvals_wrapper(&return_values[1]);

			jni_class cls(env, ctxt->cls);
			cls.call(dummy, retvals_wrapper, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);
		}
	} catch(std::runtime_error& err)
	{
		handle_err(out_err, err.what());
	} catch(...)
	{
		handle_err(out_err, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
void jni_xcall_no_params_no_ret(void* context, char** out_err)
{
	try
	{
		JNIEnv* env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&] { release_environment(); });

		openjdk_context* ctxt = (openjdk_context*) context;
		if(ctxt->field)// if field
		{
			if(ctxt->is_getter)
			{
				throw std::runtime_error("wrong xcall for getter");
			}
			else// setter
			{
				throw std::runtime_error("wrong xcall for setter");
			}
		}
		else// callable
		{
			cdts_java_wrapper dummy(nullptr);

			jni_class cls(env, ctxt->cls);
			cls.call(dummy, dummy, ctxt->field_or_return_type, ctxt->instance_required, ctxt->constructor, ctxt->any_type_indices, ctxt->method);
		}
	} catch(std::runtime_error& err)
	{
		handle_err(out_err, err.what());
	} catch(...)
	{
		handle_err(out_err, "Unknown error occurred");
	}
}
//--------------------------------------------------------------------
xcall* load_entity(const char* module_path, const char* entity_path, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err)
{
	xcall* res = nullptr;
	try
	{
		metaffi::utils::entity_path_parser fp(entity_path);
		if(!fp.contains("class"))
		{
			throw std::runtime_error("Missing class in function path");
		}

		JNIEnv* env;
		auto release_environment = pjvm->get_environment(&env);
		metaffi::utils::scope_guard sg([&] { release_environment(); });

		std::string classToLoad = fp["class"];

		jni_class_loader cloader(env, std::string(module_path));

		openjdk_context* ctxt = new openjdk_context();

		jni_class loaded_class = cloader.load_class(classToLoad);
		ctxt->cls = (jclass) loaded_class;
		ctxt->instance_required = fp.contains("instance_required");

		void* entrypoint = nullptr;

		// set in context and parameters indices of "any_type"
		for(uint8_t i = 0; i < params_count; i++)
		{
			if(params_types[i].type == metaffi_any_type)
			{
				ctxt->any_type_indices.insert(i);
			}
		}

		if(fp.contains("field"))
		{
			if(fp.contains("getter"))
			{
				ctxt->is_getter = true;

				if(retval_count == 0)
				{
					throw std::runtime_error("return type of getter is not specified");
				}

				ctxt->field = loaded_class.load_field(fp["field"], argument_definition(retvals_types[0]), fp.contains("instance_required"));
				ctxt->field_or_return_type = retvals_types[0];
				entrypoint = fp.contains("instance_required") ? (void*) jni_xcall_params_ret : (void*) jni_xcall_no_params_ret;
			}
			else if(fp.contains("setter"))
			{
				ctxt->is_getter = false;
				argument_definition argtype = fp.contains("instance_required") ? argument_definition(params_types[1]) : argument_definition(params_types[0]);
				ctxt->field = loaded_class.load_field(fp["field"], argtype, fp.contains("instance_required"));
				ctxt->field_or_return_type = argtype.type;
				entrypoint = (void*) jni_xcall_params_no_ret;
			}
			else
			{
				throw std::runtime_error("field in function path expects either getter or setter");
			}
		}
		else if(fp.contains("callable"))
		{
			if(retval_count > 1)
			{
				throw std::runtime_error("Java does not support multiple return values");
			}

			std::vector<argument_definition> parameters;
			if(params_count > 0)
			{
				int i = ctxt->instance_required ? 1 : 0;// if instance required, skip "this"
				for(; i < params_count; i++)
				{
					parameters.emplace_back(params_types[i]);
				}
			}

			ctxt->method = loaded_class.load_method(fp["callable"],
			                                        retval_count == 0 ? argument_definition() : argument_definition(retvals_types[0]),
			                                        parameters,
			                                        fp.contains("instance_required"));

			ctxt->constructor = fp["callable"] == "<init>";
			ctxt->field_or_return_type = retval_count == 0 ? metaffi_type_info{metaffi_null_type} : retvals_types[0];

			if(params_count == 0 && retval_count == 0)
			{
				entrypoint = (void*) jni_xcall_no_params_no_ret;
			}
			else if(params_count > 0 && retval_count == 0)
			{
				entrypoint = (void*) jni_xcall_params_no_ret;
			}
			else if(params_count == 0 && retval_count > 0)
			{
				entrypoint = (void*) jni_xcall_no_params_ret;
			}
			else if(params_count > 0 && retval_count > 0)
			{
				entrypoint = (void*) jni_xcall_params_ret;
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

		res = new xcall(entrypoint, ctxt);
	}
	catch_and_fill(err);

	return res;
}
//--------------------------------------------------------------------
xcall* make_callable(void* make_callable_context, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err)
{
	xcall* res = nullptr;
	try
	{
		openjdk_context* ctxt = (openjdk_context*) make_callable_context;

		void* entrypoint = nullptr;

		// set in context and parameters indices of "any_type"
		for(uint8_t i = 0; i < params_count; i++)
		{
			if(params_types[i].type == metaffi_any_type)
			{
				ctxt->any_type_indices.insert(i);
			}
		}

		if(retval_count > 1)
		{
			throw std::runtime_error("Java does not support multiple return values");
		}

		std::vector<argument_definition> parameters;
		if(params_count > 0)
		{
			int i = ctxt->instance_required ? 1 : 0;// if instance required, skip "this"
			for(; i < params_count; i++)
			{
				parameters.emplace_back(params_types[i]);
			}
		}

		ctxt->field_or_return_type = retval_count == 0 ? metaffi_type_info{metaffi_null_type} : retvals_types[0];

		entrypoint = (params_count == 0 && retval_count == 0) ? (void*) jni_xcall_no_params_no_ret : (params_count > 0 && retval_count == 0) ? (void*) jni_xcall_params_no_ret
		                                                                                     : (params_count == 0 && retval_count > 0)       ? (void*) jni_xcall_no_params_ret
		                                                                                     : (params_count > 0 && retval_count > 0)        ? (void*) jni_xcall_params_ret
		                                                                                                                                     : nullptr;

		if(!entrypoint)
		{
			std::stringstream ss;
			ss << "Failed to detect suitable entrypoint. Params count: " << params_count << ". Retvals count: " << retval_count;
			throw std::runtime_error(ss.str().c_str());
		}

		res = new xcall(entrypoint, ctxt);
	}
	catch_and_fill(err);

	return res;
}
//--------------------------------------------------------------------
void free_xcall(xcall* pxcall, char** err)
{
	try
	{
		if(pxcall)
		{
			delete(openjdk_context*) pxcall->pxcall_and_context[1];
			delete pxcall;
		}
	}
	catch_and_fill(err);
}
//--------------------------------------------------------------------
