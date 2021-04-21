#include "openffi_XLLR.h"
#include <utils/xllr_api_wrapper.h>

// JNI to call XLLR from java

using namespace openffi::utils;

std::unique_ptr<xllr_api_wrapper> xllr;

//--------------------------------------------------------------------
JNIEXPORT jstring JNICALL Java_openffi_XLLR_init(JNIEnv* env, jobject obj)
{
	try
	{
		xllr = std::make_unique<xllr_api_wrapper>();
		return env->NewStringUTF("");
	}
	catch(std::exception& err)
	{
		return env->NewStringUTF(err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT jstring JNICALL Java_openffi_XLLR_load_1runtime_1plugin(JNIEnv* env, jobject obj, jstring runtime_plugin)
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
		jstring err_ret = env->NewStringUTF(out_err_buf);
		free(out_err_buf);
		return err_ret;
	}
	
	return env->NewStringUTF("");
}
//--------------------------------------------------------------------
JNIEXPORT jstring JNICALL Java_openffi_XLLR_free_1runtime_1plugin(JNIEnv* env, jobject obj, jstring runtime_plugin)
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
		jstring err_ret = env->NewStringUTF(out_err_buf);
		free(out_err_buf);
		return err_ret;
	}
	
	return env->NewStringUTF("");
}
//--------------------------------------------------------------------
JNIEXPORT jobject JNICALL Java_openffi_XLLR_load_1function(JNIEnv* env, jobject obj, jstring runtime_plugin, jstring function_path)
{
	const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
	jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
	
	const char* str_function_path = env->GetStringUTFChars(function_path, nullptr);
	jsize str_function_path_len = env->GetStringLength(function_path);
	
	char* out_err_buf = nullptr;
	uint32_t out_err_len = 0;
	int64_t function_id = xllr->load_function(str_runtime_plugin, str_runtime_plugin_len, str_function_path, str_function_path_len, &out_err_buf, &out_err_len);
	
	// release runtime_plugin
	env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
	env->ReleaseStringUTFChars(function_path, str_function_path);
	
	// set to out_err
	if(function_id == -1)
	{
		jstring err_ret = env->NewStringUTF(out_err_buf);
		free(out_err_buf);
		return err_ret;
	}
	
	jclass jclassLong = env->FindClass("J");
	if(!jclassLong)
	{
		jstring err_ret = env->NewStringUTF("Failed to find Long class");
		return err_ret;
	}
	
	jmethodID cstr = env->GetMethodID(jclassLong, "<init>", "(J)J");
	if(!cstr)
	{
		jstring err_ret = env->NewStringUTF("Failed to get Long class constructor");
		return err_ret;
	}
	
	jobject res = env->NewObject(jclassLong, cstr, (jlong)function_id);
	if(!res)
	{
		jstring err_ret = env->NewStringUTF("Failed to get Long class with function id");
		return err_ret;
	}
	
	return res;
}
//--------------------------------------------------------------------
JNIEXPORT jstring JNICALL Java_openffi_XLLR_free_1function(JNIEnv* env, jobject obj, jstring runtime_plugin, jlong function_id)
{
	const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
	jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
	
	char* out_err_buf = nullptr;
	uint32_t out_err_len = 0;
	xllr->free_function(str_runtime_plugin, str_runtime_plugin_len, function_id, &out_err_buf, &out_err_len);
	
	// release runtime_plugin
	env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
	
	// set to out_err
	if(out_err_buf)
	{
		jstring err_ret = env->NewStringUTF(out_err_buf);
		free(out_err_buf);
		return err_ret;
	}
	
	return env->NewStringUTF("");
}
//--------------------------------------------------------------------
JNIEXPORT jstring JNICALL Java_openffi_XLLR_call(JNIEnv* env, jobject obj,
                                                 jstring runtime_plugin,
                                                 jlong function_id,
                                                 jbyteArray in_params,
                                                 jobject out_result)
{
	const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
	jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
	
	jbyte* arr_in_params = env->GetByteArrayElements(in_params, nullptr);
	jsize arr_in_params_len = env->GetArrayLength(in_params);
	
	unsigned char* out_params = nullptr;
	uint64_t out_params_len = 0;
	unsigned char* out_ret = nullptr;
	uint64_t out_ret_len = 0;
	uint8_t out_is_err = 0;
	xllr->call(str_runtime_plugin, str_runtime_plugin_len,
	           function_id,
	           (unsigned char*)arr_in_params, arr_in_params_len,
	           &out_params, &out_params_len,
	           &out_ret, &out_ret_len,
	           &out_is_err);
	
	
	env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
	env->ReleaseByteArrayElements(in_params, arr_in_params, 0);
	
	if(out_is_err)
	{
		jstring err_ret = env->NewStringUTF(std::string((const char*)out_ret, out_ret_len).c_str());
		free(out_ret);
		return err_ret;
	}
	
	if(out_params || out_ret)
	{
		jclass CallResult_class = env->GetObjectClass(out_result);
		
		if(out_params)
		{
			jfieldID id = env->GetFieldID(CallResult_class, "out_params", "Lopenffi/CallResult/[B;");
			jobject call_result_out_params = env->GetObjectField(out_result, id);
			env->SetByteArrayRegion((jbyteArray)call_result_out_params, 0, out_params_len, (const jbyte*)out_params);
			
			// TODO: release call_result_out_params?
		}
		
		if(out_ret)
		{
			jfieldID id = env->GetFieldID(CallResult_class, "out_ret", "Lopenffi/CallResult/[B;");
			jobject call_result_out_ret = env->GetObjectField(out_result, id);
			env->SetByteArrayRegion((jbyteArray)call_result_out_ret, 0, out_ret_len, (const jbyte*)out_ret);
			
			// TODO: release call_result_out_ret?
		}
	}
	
	return env->NewStringUTF("");
}
//--------------------------------------------------------------------