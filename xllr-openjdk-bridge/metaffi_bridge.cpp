#ifdef _MSC_VER
#include <corecrt.h> // https://www.reddit.com/r/cpp_questions/comments/qpo93t/error_c2039_invalid_parameter_is_not_a_member_of/
#endif
#include "metaffi_bridge.h"
#include <runtime/cdts_wrapper.h>
#include <runtime/cdt_capi_loader.h>
#include <utils/foreign_function.h>
#include <utils/xllr_api_wrapper.h>
#include <runtime/metaffi_primitives.h>
#include "../runtime/objects_table.h"

// JNI to call XLLR from java

using namespace metaffi::utils;


//--------------------------------------------------------------------

std::unique_ptr<xllr_api_wrapper> xllr;

//--------------------------------------------------------------------
// https://stackoverflow.com/questions/230689/best-way-to-throw-exceptions-in-jni-code
bool throwMetaFFIException( JNIEnv* env,  const char* message )
{
	jclass metaffiException = env->FindClass( "metaffi/MetaFFIException" );
	if(!metaffiException)
	{
		jclass noSuchClass = env->FindClass( "java/lang/NoClassDefFoundError" );
		return env->ThrowNew( noSuchClass, "Cannot find MetaFFIException Class" ) == 0;
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
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_load_1function(JNIEnv* env, jobject obj, jstring runtime_plugin, jstring module_path, jstring function_path, jbyte params_count, jbyte retval_count)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		
		const char* str_module_path = env->GetStringUTFChars(module_path, nullptr);
		jsize str_module_path_len = env->GetStringLength(module_path);
		
		const char* str_function_path = env->GetStringUTFChars(function_path, nullptr);
		jsize str_function_path_len = env->GetStringLength(function_path);
		
		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;
		void* pff = nullptr; // TODO //xllr->load_function(str_runtime_plugin, str_runtime_plugin_len, str_module_path, str_module_path_len, str_function_path, str_function_path_len, nullptr, params_count, retval_count, &out_err_buf, &out_err_len);
		
		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		env->ReleaseStringUTFChars(function_path, str_function_path);
		
		// set to out_err
		if(!pff)
		{
			throwMetaFFIException(env, out_err_buf);
			//free(out_err_buf);
			return -1;
		}
		
		return (jlong)pff;
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
		
		
		((pforeign_function_entrypoint_signature_params_ret) pff)(nullptr, (cdts*) xcall_params, &out_err_buf, &out_err_len);
		
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
	
	((pforeign_function_entrypoint_signature_no_params_ret)pff)(nullptr, (cdts*)xcall_params, &out_err_buf, &out_err_len);
	
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
		
		((pforeign_function_entrypoint_signature_params_no_ret) pff)(nullptr, (cdts*) xcall_params, &out_err_buf, &out_err_len);
		
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
		
		((pforeign_function_entrypoint_signature_no_params_no_ret) pff)(nullptr, &out_err_buf, &out_err_len);
		
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
	env->DeleteGlobalRef((jobject)phandle);
	openjdk_objects_table::instance().remove((jobject)phandle);
}
//--------------------------------------------------------------------