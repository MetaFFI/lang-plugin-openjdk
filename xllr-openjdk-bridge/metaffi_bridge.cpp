#include "metaffi_bridge.h"
#include <runtime/cdts_wrapper.h>
#include <runtime/cdt_capi_loader.h>
#include <utils/foreign_function.h>
#include <utils/xllr_api_wrapper.h>
#include "../runtime/cdts_java.h"
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
	
	return env->ThrowNew( metaffiException, message ) == 0;
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_init(JNIEnv* env, jclass obj)
{
	try
	{
		xllr = std::make_unique<xllr_api_wrapper>();

#define load_class_methods(type_name, fq_type_name, sig_cstr, array_sig) \
		cdts_java::type_name##_class = env->FindClass( #fq_type_name ); \
		if(!cdts_java::type_name##_class){ throw std::runtime_error("Failed to find class" #fq_type_name); } \
		cdts_java::type_name##_constructor = env->GetMethodID(cdts_java::type_name##_class, "<init>", #sig_cstr ); \
		if(!cdts_java::type_name##_constructor){ throw std::runtime_error("Failed to find method ID of " #fq_type_name " constructor with signature " #sig_cstr); } \
		cdts_java::type_name##_array_class = env->FindClass( #array_sig ); \
		if(!cdts_java::type_name##_array_class){ throw std::runtime_error("Failed to find class " #array_sig ); }

#define load_primitive_class_methods(type_name, fq_type_name, sig_cstr, sig_val, get_value, array_sig) \
        load_class_methods(type_name, fq_type_name, sig_cstr, array_sig); \
		cdts_java::type_name##_get_value = env->GetMethodID(cdts_java::type_name##_class, #get_value, #sig_val ); \
		if(!cdts_java::type_name##_get_value){ throw std::runtime_error("Failed to find method ID of " #fq_type_name " get value with signature " #sig_val); }

		
		load_primitive_class_methods(byte, java/lang/Byte, (B)V, ()B, byteValue, [B);
		load_primitive_class_methods(short, java/lang/Short, (S)V, ()S, shortValue, [S);
		load_primitive_class_methods(int, java/lang/Integer, (I)V, ()I, intValue, [I);
		load_primitive_class_methods(long, java/lang/Long, (J)V, ()J, longValue, [J);
		load_primitive_class_methods(float, java/lang/Float, (F)V, ()F, floatValue, [F);
		load_primitive_class_methods(double, java/lang/Double, (D)V, ()D, doubleValue, [D);
		load_primitive_class_methods(char, java/lang/Character, (C)V, ()C, charValue, [C);
		load_primitive_class_methods(boolean, java/lang/Boolean, (Z)V, ()Z, booleanValue, [Z);
		load_class_methods(string, java/lang/String, (Ljava/lang/String;)V, [Ljava/lang/String;);
		
		cdts_java::object_class = env->FindClass("java/lang/Object");
		if (!cdts_java::object_class) { throw std::runtime_error("Failed to find class" "java/lang/Object"); }
		cdts_java::object_array_class = env->FindClass("[Ljava/lang/Object" ";");
		if (!cdts_java::object_array_class) { throw std::runtime_error("Failed to find class [L" "java/lang/Object" ";"); }
		
		cdts_java::metaffi_handle_class = env->FindClass( "Lmetaffi/MetaFFIHandle;" );
		if(!cdts_java::metaffi_handle_class){ throw std::runtime_error("Failed to find Lmetaffi/MetaFFIHandle;"); }
		cdts_java::metaffi_handle_constructor = env->GetMethodID(cdts_java::metaffi_handle_class, "<init>", "(J)V" );
		if(!cdts_java::metaffi_handle_constructor){ throw std::runtime_error("Failed to find method ID of metaffi/MetaFFIHandle constructor"); }
		cdts_java::metaffi_handle_get_value = env->GetMethodID(cdts_java::metaffi_handle_class, "Handle", "()J" );
		if(!cdts_java::metaffi_handle_get_value){ throw std::runtime_error("Failed to find method ID of Lmetaffi/MetaFFIHandle get value"); }
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
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
			free(out_err_buf);
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
			free(out_err_buf);
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_load_1function(JNIEnv* env, jobject obj, jstring runtime_plugin, jstring function_path, jbyte params_count, jbyte retval_count)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		
		const char* str_function_path = env->GetStringUTFChars(function_path, nullptr);
		jsize str_function_path_len = env->GetStringLength(function_path);
		
		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;
		void* pff = xllr->load_function(str_runtime_plugin, str_runtime_plugin_len, str_function_path, str_function_path_len, nullptr, params_count, retval_count, &out_err_buf, &out_err_len);
		
		// release runtime_plugin
		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);
		env->ReleaseStringUTFChars(function_path, str_function_path);
		
		// set to out_err
		if(!pff)
		{
			throwMetaFFIException(env, out_err_buf);
			free(out_err_buf);
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
			free(out_err_buf);
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
		
		((pforeign_function_entrypoint_signature_params_ret) pff)((cdts*) xcall_params, &out_err_buf, &out_err_len);
		
		if (out_err_len) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			free(out_err_buf);
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
	
	((pforeign_function_entrypoint_signature_no_params_ret)pff)((cdts*)xcall_params, &out_err_buf, &out_err_len);
	
	if(out_err_len) // throw an exception in the JVM
	{
		throwMetaFFIException(env, out_err_buf);
		free(out_err_buf);
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
		
		((pforeign_function_entrypoint_signature_params_no_ret) pff)((cdts*) xcall_params, &out_err_buf, &out_err_len);
		
		if (out_err_len) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			free(out_err_buf);
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
		
		((pforeign_function_entrypoint_signature_no_params_no_ret) pff)(&out_err_buf, &out_err_len);
		
		if (out_err_len) // throw an exception in the JVM
		{
			throwMetaFFIException(env, out_err_buf);
			free(out_err_buf);
			return;
		}
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
	}
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_java_1to_1cdts(JNIEnv* env, jobject, jlong pcdts, jobjectArray parameters, jlongArray pmetaffi_types)
{
	try
	{
		jsize len = env->GetArrayLength(parameters);
		
		cdts_java wrap(((cdt*) pcdts), len, env);
		jboolean is_copy = JNI_FALSE;
		
		jlong* pmtypes_array = env->GetLongArrayElements(pmetaffi_types, &is_copy);
		wrap.build(parameters, reinterpret_cast<metaffi_types_ptr>(pmtypes_array), len, 0);
		
		if(is_copy == JNI_TRUE){
			env->ReleaseLongArrayElements(pmetaffi_types, pmtypes_array, 0);
		}
		
		return reinterpret_cast<jlong>(pcdts);
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jobjectArray JNICALL Java_metaffi_MetaFFIBridge_cdts_1to_1java(JNIEnv* env, jobject obj, jlong pcdts, jlong len)
{
	try
	{
		cdts_java wrap(((cdt*) pcdts), len, env);
		auto r = wrap.parse();
		return r;
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return nullptr;
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