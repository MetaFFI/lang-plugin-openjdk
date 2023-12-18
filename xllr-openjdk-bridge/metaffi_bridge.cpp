#ifdef _MSC_VER
#include <corecrt.h> // https://www.reddit.com/r/cpp_questions/comments/qpo93t/error_c2039_invalid_parameter_is_not_a_member_of/
#endif
#include "metaffi_bridge.h"
#include <runtime/cdt_capi_loader.h>
#include <utils/foreign_function.h>
#include <utils/xllr_api_wrapper.h>
#include <runtime/metaffi_primitives.h>
#include <utils/scope_guard.hpp>

#include "../runtime/objects_table.h"
#include "../runtime/cdts_java_wrapper.h"
#include "../runtime/exception_macro.h"

// JNI to call XLLR from java

using namespace metaffi::utils;


//--------------------------------------------------------------------

std::unique_ptr<xllr_api_wrapper> xllr = std::make_unique<xllr_api_wrapper>();

//--------------------------------------------------------------------
// https://stackoverflow.com/questions/230689/best-way-to-throw-exceptions-in-jni-code
bool throwMetaFFIException( JNIEnv* env,  const char* message )
{
	jclass metaffiException = env->FindClass( "metaffi/MetaFFIException" );
	if(!metaffiException)
	{
		jclass noSuchClass = env->FindClass( "java/lang/NoClassDefFoundError" );
		return env->ThrowNew( noSuchClass, "Cannot find metaffi/MetaFFIException Class" ) == 0;
	}
	
	jint res = env->ThrowNew( metaffiException, message );
	
	
	return res == 0;
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_load_1runtime_1plugin(JNIEnv* env, jobject obj, jstring runtime_plugin)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		
		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;
		xllr->load_runtime_plugin(str_runtime_plugin, str_runtime_plugin_len, &out_err_buf, &out_err_len);
		
		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		
		// set to out_err
		if(out_err_buf)
		{
			throwMetaFFIException(env, out_err_buf);
			//free(out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_free_1runtime_1plugin(JNIEnv* env, jobject obj, jstring runtime_plugin)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		
		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;
		xllr->free_runtime_plugin(str_runtime_plugin, str_runtime_plugin_len, &out_err_buf, &out_err_len);
		
		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		
		// set to out_err
		if(out_err_buf)
		{
			throwMetaFFIException(env, out_err_buf);
			//free(out_err_buf);
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
jclass metaFFITypeWithAliasClass = nullptr;
jfieldID typeFieldID = nullptr;
jfieldID aliasFieldID = nullptr;
metaffi_types_with_alias_ptr convert_MetaFFITypeWithAlias_array_to_metaffi_type_with_alias(JNIEnv* env, jobjectArray inputArray)
{
	// Get the length of the input array
	jsize length = env->GetArrayLength(inputArray);
	check_and_throw_jvm_exception(env, true);
	// Allocate the output array
	metaffi_types_with_alias_ptr outputArray = new metaffi_type_with_alias[length];

	// Get the MetaFFITypeWithAlias class
	if(!metaFFITypeWithAliasClass)
	{
		metaFFITypeWithAliasClass = env->FindClass("metaffi/MetaFFITypeWithAlias");
		check_and_throw_jvm_exception(env, true);
	}

	// Get the field IDs
	if(!typeFieldID)
	{
		typeFieldID = env->GetFieldID(metaFFITypeWithAliasClass, "value", "J"); // long
		check_and_throw_jvm_exception(env, true);
	}

	if(!aliasFieldID)
	{
		aliasFieldID = env->GetFieldID(metaFFITypeWithAliasClass, "alias", "Ljava/lang/String;"); // String
		check_and_throw_jvm_exception(env, true);
	}

	// Iterate over the input array
	for (jsize i = 0; i < length; i++)
	{
		// Get the current MetaFFITypeWithAlias object
		jobject metaFFITypeWithAliasObject = env->GetObjectArrayElement(inputArray, i);
		check_and_throw_jvm_exception(env, true);

		// Get the type and alias
		jlong type = env->GetLongField(metaFFITypeWithAliasObject, typeFieldID);
		check_and_throw_jvm_exception(env, true);
		jstring aliasJString = (jstring)env->GetObjectField(metaFFITypeWithAliasObject, aliasFieldID);
		check_and_throw_jvm_exception(env, true);

		if(aliasJString)
		{
			const char* aliasCString = env->GetStringUTFChars(aliasJString, nullptr);
			check_and_throw_jvm_exception(env, true);
			jsize len = env->GetStringUTFLength(aliasJString);
			check_and_throw_jvm_exception(env, true);
			outputArray[i].alias = (char*)calloc(len + 1, 1);
			std::copy_n(aliasCString, len, outputArray[i].alias);
			outputArray[i].alias_length = len;

			env->ReleaseStringUTFChars(aliasJString, aliasCString);
			check_and_throw_jvm_exception(env, true);
		}
		else
		{
			outputArray[i].alias = nullptr;
			outputArray[i].alias_length = 0;
		}

		// Set the output array values
		outputArray[i].type = type;
	}

	// Return the output array as a jlong (this is actually a pointer)
	return reinterpret_cast<metaffi_types_with_alias_ptr>(outputArray);
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_load_1function(JNIEnv* env, jobject obj, jstring runtime_plugin, jstring module_path, jstring function_path, jobjectArray parameters_types, jobjectArray retval_types)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		check_and_throw_jvm_exception(env, str_runtime_plugin);

		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		check_and_throw_jvm_exception(env, str_runtime_plugin_len);

		const char* str_module_path = env->GetStringUTFChars(module_path, nullptr);
		check_and_throw_jvm_exception(env, str_module_path);

		jsize str_module_path_len = env->GetStringLength(module_path);
		check_and_throw_jvm_exception(env, str_module_path_len);

		const char* str_function_path = env->GetStringUTFChars(function_path, nullptr);
		check_and_throw_jvm_exception(env, str_function_path);

		jsize str_function_path_len = env->GetStringLength(function_path);
		check_and_throw_jvm_exception(env, true);

		jsize params_count = parameters_types == nullptr ? 0 : env->GetArrayLength(parameters_types);
		check_and_throw_jvm_exception(env, true);

		jsize retval_count = retval_types == nullptr ? 0 : env->GetArrayLength(retval_types);
		check_and_throw_jvm_exception(env, true);

		metaffi_types_with_alias_ptr pparams_types = parameters_types == nullptr ?
													nullptr :
													convert_MetaFFITypeWithAlias_array_to_metaffi_type_with_alias(env, parameters_types);

		metaffi_types_with_alias_ptr pretval_types = retval_types == nullptr ?
		                                             nullptr :
		                                             convert_MetaFFITypeWithAlias_array_to_metaffi_type_with_alias(env, retval_types);
		
		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;

		void* xcall_and_context = xllr->load_function(str_runtime_plugin, str_runtime_plugin_len,
										str_module_path, str_module_path_len,
										str_function_path, str_function_path_len,
										pparams_types, pretval_types,
										(uint8_t)params_count, (uint8_t)retval_count, &out_err_buf, &out_err_len);

		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		env->ReleaseStringUTFChars(function_path, str_function_path);

		if(params_count > 0)
		{
			for(int i=0 ; i<params_count ; i++)
			{
				if(pparams_types[i].alias_length > 0)
				{
					free(pparams_types[i].alias);
				}
			}
			delete[] pparams_types;
		}

		if(retval_count > 0)
		{
			for(int i=0 ; i<retval_count ; i++)
			{
				if(pretval_types[i].alias_length > 0)
				{
					free(pretval_types[i].alias);
				}
			}
			delete[] pretval_types;
		}

		// set to out_err
		if(!xcall_and_context)
		{
			throwMetaFFIException(env, out_err_buf);
			return -1;
		}

		return (jlong)xcall_and_context;
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_free_1function(JNIEnv* env, jobject obj, jstring runtime_plugin, jlong function_id)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		
		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;
		xllr->free_function(str_runtime_plugin, str_runtime_plugin_len, (void*) obj, &out_err_buf, &out_err_len);
		
		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		
		// set to out_err
		if (out_err_buf)
		{
			throwMetaFFIException(env, out_err_buf);
			//free(out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1params_1ret(JNIEnv* env, jobject obj, jlong pff, jlong xcall_params)
{
	try
	{
		char* out_err_buf = nullptr;
		uint64_t out_err_len = 0;
		
		if(!pff)
		{
			throwMetaFFIException(env, "internal error. pff is null");
			return;
		}
		
		void** ppff = (void**)pff;
		
		((pforeign_function_entrypoint_signature_params_ret) ppff[0])(ppff[1], (cdts*) xcall_params, &out_err_buf, &out_err_len);
		
		if (out_err_len) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			//free(out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
	
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1no_1params_1ret(JNIEnv* env, jobject obj, jlong pff, jlong xcall_params)
{
	char* out_err_buf = nullptr;
	uint64_t out_err_len = 0;
	
	if(!pff)
	{
		throwMetaFFIException(env, "internal error. pff is null");
		return;
	}
	
	void** ppff = (void**)pff;
	((pforeign_function_entrypoint_signature_no_params_ret)ppff[0])(ppff[1], (cdts*)xcall_params, &out_err_buf, &out_err_len);
	
	if(out_err_len) // throw an exception in the JVM
	{
		throwMetaFFIException(env, out_err_buf);
		//free(out_err_buf);
		return;
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1params_1no_1ret(JNIEnv* env, jobject obj, jlong pff, jlong xcall_params)
{
	try
	{
		char* out_err_buf = nullptr;
		uint64_t out_err_len = 0;
		
		if(!pff)
		{
			throwMetaFFIException(env, "internal error. pff is null");
			return;
		}
		
		void** ppff = (void**)pff;
		((pforeign_function_entrypoint_signature_params_no_ret) ppff[0])(ppff[1], (cdts*) xcall_params, &out_err_buf, &out_err_len);
		
		if (out_err_len) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			//free(out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1no_1params_1no_1ret(JNIEnv* env, jobject obj, jlong pff)
{
	try
	{
		char* out_err_buf = nullptr;
		uint64_t out_err_len = 0;
		
		if(!pff)
		{
			throwMetaFFIException(env, "internal error. pff is null");
			return;
		}
		
		void** ppff = (void**)pff;
		((pforeign_function_entrypoint_signature_no_params_no_ret) ppff[0])(ppff[1], &out_err_buf, &out_err_len);
		
		if (out_err_len) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			
			//free(out_err_buf);
			
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_alloc_1cdts(JNIEnv* env, jobject, jbyte params_count, jbyte retval_count)
{
	try
	{
		return reinterpret_cast<jlong>(xllr_alloc_cdts_buffer(params_count, retval_count));
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_get_1pcdt(JNIEnv* env, jobject, jlong pcdts, jbyte index)
{
	try
	{
		return reinterpret_cast<jlong>(reinterpret_cast<cdts*>(pcdts)[index].pcdt);
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jobject JNICALL Java_metaffi_MetaFFIBridge_get_1object(JNIEnv* env, jobject, jlong phandle)
{
	if(openjdk_objects_table::instance().contains((jobject)phandle)){
		throwMetaFFIException(env, "Object is not found in Objects Table");
	}
	
	return (jobject)phandle;
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_remove_1object (JNIEnv* env, jobject, jlong phandle)
{
	openjdk_objects_table::instance().remove(env, (jobject)phandle);
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_java_1to_1cdts(JNIEnv* env, jobject, jlong pcdts, jobjectArray parameters, jlongArray types)
{
	try
	{
		jsize l = env->GetArrayLength(parameters);
		cdts_java_wrapper wrapper((cdt*)pcdts, l);

		jlong* types_elements = env->GetLongArrayElements(types, nullptr);
		check_and_throw_jvm_exception(env, types_elements);
		
		metaffi::utils::scope_guard sg([&](){ env->ReleaseLongArrayElements(types, types_elements, 0); });
		
		for(jsize i=0 ; i < l ; i++)
		{
			jvalue cur_object;
			cur_object.l = env->GetObjectArrayElement(parameters, i);
			check_and_throw_jvm_exception(env, true);
			metaffi_type type_to_expect = (types_elements[i] & metaffi_array_type) == 0 ? metaffi_handle_type : types_elements[i];
			wrapper.from_jvalue(env, cur_object, type_to_expect, i);
			wrapper.switch_to_primitive(env, i, types_elements[i]);
		}
		

		return (jlong)wrapper.get_cdts();
	}
	catch(std::exception& exp)
	{
		throwMetaFFIException(env, exp.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jobjectArray JNICALL Java_metaffi_MetaFFIBridge_cdts_1to_1java(JNIEnv* env, jobject, jlong pcdts, jlong length)
{
	try
	{
		cdts_java_wrapper wrapper((cdt*)pcdts, length);
		jobjectArray arr = env->NewObjectArray(length, env->FindClass("Ljava/lang/Object;"), nullptr);
		for (int i=0 ; i<length ; i++)
		{
			wrapper.switch_to_object(env, i);
			jvalue j = wrapper.to_jvalue(env, i);
			env->SetObjectArrayElement(arr, i, j.l);
			check_and_throw_jvm_exception(env, true);

			if(env->GetObjectRefType(j.l) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(j.l); // delete the global reference
			}

		}

		return arr;
	}
	catch(std::exception& exp)
	{
		throwMetaFFIException(env, exp.what());
		return nullptr;
	}
}
//--------------------------------------------------------------------
