#ifdef _MSC_VER
#include <corecrt.h> // https://www.reddit.com/r/cpp_questions/comments/qpo93t/error_c2039_invalid_parameter_is_not_a_member_of/
#endif
#include "metaffi_bridge.h"
#include <runtime/xllr_capi_loader.h>
#include <utils/foreign_function.h>
#include <utils/xllr_api_wrapper.h>
#include <runtime/metaffi_primitives.h>
#include <utils/scope_guard.hpp>

#include "../runtime/objects_table.h"
#include "../runtime/cdts_java_wrapper.h"
#include "../runtime/exception_macro.h"
#include "../runtime/contexts.h"

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
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_load_1runtime_1plugin(JNIEnv* env, jclass , jstring runtime_plugin)
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
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_free_1runtime_1plugin(JNIEnv* env, jclass , jstring runtime_plugin)
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
jclass MetaFFITypeInfoClass = nullptr;
jfieldID typeFieldID = nullptr;
jfieldID aliasFieldID = nullptr;
metaffi_type_info* convert_MetaFFITypeInfo_array_to_metaffi_type_with_alias(JNIEnv* env, jobjectArray inputArray)
{
	// Get the length of the input array
	jsize length = env->GetArrayLength(inputArray);
	check_and_throw_jvm_exception(env, true);
	// Allocate the output array
	metaffi_type_info* outputArray = new metaffi_type_info[length];

	// Get the MetaFFITypeInfo class
	if(!MetaFFITypeInfoClass)
	{
		MetaFFITypeInfoClass = env->FindClass("metaffi/MetaFFITypeInfo");
		check_and_throw_jvm_exception(env, true);
	}

	// Get the field IDs
	if(!typeFieldID)
	{
		typeFieldID = env->GetFieldID(MetaFFITypeInfoClass, "value", "J"); // long
		check_and_throw_jvm_exception(env, true);
	}

	if(!aliasFieldID)
	{
		aliasFieldID = env->GetFieldID(MetaFFITypeInfoClass, "alias", "Ljava/lang/String;"); // String
		check_and_throw_jvm_exception(env, true);
	}

	// Iterate over the input array
	for (jsize i = 0; i < length; i++)
	{
		// Get the current MetaFFITypeInfo object
		jobject MetaFFITypeInfoObject = env->GetObjectArrayElement(inputArray, i);
		check_and_throw_jvm_exception(env, true);

		// Get the type and alias
		jlong type = env->GetLongField(MetaFFITypeInfoObject, typeFieldID);
		check_and_throw_jvm_exception(env, true);
		jstring aliasJString = (jstring)env->GetObjectField(MetaFFITypeInfoObject, aliasFieldID);
		check_and_throw_jvm_exception(env, true);

		if(aliasJString)
		{
			const char* aliasCString = env->GetStringUTFChars(aliasJString, nullptr);
			check_and_throw_jvm_exception(env, true);
			jsize len = env->GetStringUTFLength(aliasJString);
			check_and_throw_jvm_exception(env, true);
			outputArray[i].alias = new char[len + 1];
			std::copy_n(aliasCString, len, outputArray[i].alias);
			outputArray[i].alias[len] = u8'\0';
			
			env->ReleaseStringUTFChars(aliasJString, aliasCString);
			check_and_throw_jvm_exception(env, true);
		}
		else
		{
			outputArray[i].alias = nullptr;
		}

		// Set the output array values
		outputArray[i].type = type;
	}

	// Return the output array as a jlong (this is actually a pointer)
	return reinterpret_cast<metaffi_type_info*>(outputArray);
}
//--------------------------------------------------------------------
jmethodID get_method_id_from_Method(JNIEnv* env, jclass methodClass, jobject methodObject, jstring jniSignature, jboolean& isStatic)
{
	// Get Method and Modifier classes and method IDs we're going to need
	jclass classMethod = env->FindClass("java/lang/reflect/Method");
	jclass classModifier = env->FindClass("java/lang/reflect/Modifier");
	jmethodID midGetDeclaringClass = env->GetMethodID(classMethod, "getDeclaringClass", "()Ljava/lang/Class;");
	jmethodID midGetName = env->GetMethodID(classMethod, "getName", "()Ljava/lang/String;");
	jmethodID midGetModifiers = env->GetMethodID(classMethod, "getModifiers", "()I");
	jmethodID midIsStatic = env->GetStaticMethodID(classModifier, "isStatic", "(I)Z");

	// Call getDeclaringClass, getName, getModifiers methods
	jobject declaringClassObject = env->CallObjectMethod(methodObject, midGetDeclaringClass);
	jstring nameObject = (jstring)env->CallObjectMethod(methodObject, midGetName);
	jint modifiers = env->CallIntMethod(methodObject, midGetModifiers);

	// Call Modifier.isStatic method
	isStatic = env->CallStaticBooleanMethod(classModifier, midIsStatic, modifiers);

	// Get the declaring class
	jclass declaringClass = (jclass)env->NewLocalRef(declaringClassObject);

	// Get the method name
	const char* name = env->GetStringUTFChars(nameObject, 0);

	// Get the JNI signature
	const char* signature = env->GetStringUTFChars(jniSignature, 0);

	// Get the method ID
	jmethodID mid;
	if (isStatic == JNI_TRUE) {
		mid = env->GetStaticMethodID(declaringClass, name, signature);
	} else {
		mid = env->GetMethodID(declaringClass, name, signature);
	}

	// Clean up local references and release the strings
	env->DeleteLocalRef(declaringClass);
	env->ReleaseStringUTFChars(nameObject, name);
	env->ReleaseStringUTFChars(jniSignature, signature);

	return mid;
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_load_1callable(JNIEnv* env, jclass , jstring runtime_plugin, jobject method, jstring method_jni_signature, jobjectArray parameters_types, jobjectArray retval_types)
{
	try
	{
		const char* str_runtime_plugin = env->GetStringUTFChars(runtime_plugin, nullptr);
		check_and_throw_jvm_exception(env, str_runtime_plugin);

		jsize str_runtime_plugin_len = env->GetStringLength(runtime_plugin);
		check_and_throw_jvm_exception(env, str_runtime_plugin_len);

		jclass cls = env->GetObjectClass(method);
		check_and_throw_jvm_exception(env, true);

		jboolean out_is_static = JNI_FALSE;
		jmethodID method_id = get_method_id_from_Method(env, cls, method, method_jni_signature, out_is_static);


		jsize params_count = parameters_types == nullptr ? 0 : env->GetArrayLength(parameters_types);
		check_and_throw_jvm_exception(env, true);

		jsize retval_count = retval_types == nullptr ? 0 : env->GetArrayLength(retval_types);
		check_and_throw_jvm_exception(env, true);

		metaffi_type_info* pparams_types = parameters_types == nullptr ?
													nullptr :
													convert_MetaFFITypeInfo_array_to_metaffi_type_with_alias(env, parameters_types);

		metaffi_type_info* pretval_types = retval_types == nullptr ?
		                                             nullptr :
		                                             convert_MetaFFITypeInfo_array_to_metaffi_type_with_alias(env, retval_types);

		char* out_err_buf = nullptr;
		uint32_t out_err_len = 0;

		openjdk_context* pctxt = new openjdk_context();
		pctxt->cls = cls;
		pctxt->method = method_id;
		pctxt->instance_required = out_is_static == JNI_FALSE;
		pctxt->constructor = false;

		void* xcall_and_context = xllr->make_callable(str_runtime_plugin, str_runtime_plugin_len, (void*)pctxt,
										pparams_types, pretval_types,
										(uint8_t)params_count, (uint8_t)retval_count, &out_err_buf, &out_err_len);

		env->ReleaseStringUTFChars(runtime_plugin, str_runtime_plugin);

		if(params_count > 0)
		{
			for(int i=0 ; i<params_count ; i++)
			{
				if(pparams_types[i].is_free_alias && pparams_types[i].alias)
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
				if(pretval_types[i].is_free_alias && pretval_types[i].alias)
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
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_load_1function(JNIEnv* env, jclass , jstring runtime_plugin, jstring module_path, jstring function_path, jobjectArray parameters_types, jobjectArray retval_types)
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

		metaffi_type_info* pparams_types = parameters_types == nullptr ?
													nullptr :
													convert_MetaFFITypeInfo_array_to_metaffi_type_with_alias(env, parameters_types);

		metaffi_type_info* pretval_types = retval_types == nullptr ?
		                                             nullptr :
		                                             convert_MetaFFITypeInfo_array_to_metaffi_type_with_alias(env, retval_types);

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
				if(pparams_types[i].is_free_alias && pparams_types[i].alias)
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
				if(pretval_types[i].is_free_alias && pretval_types[i].alias)
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
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_free_1function(JNIEnv* env, jclass , jstring runtime_plugin, jlong function_id)
{

}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1params_1ret(JNIEnv* env, jclass , jlong pff, jlong xcall_params)
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
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1no_1params_1ret(JNIEnv* env, jclass , jlong pff, jlong xcall_params)
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
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1params_1no_1ret(JNIEnv* env, jclass , jlong pff, jlong xcall_params)
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
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1no_1params_1no_1ret(JNIEnv* env, jclass , jlong pff)
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
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_alloc_1cdts(JNIEnv* env, jclass, jbyte params_count, jbyte retval_count)
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
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_get_1pcdt(JNIEnv* env, jclass, jlong pcdts, jbyte index)
{
	try
	{
		return reinterpret_cast<jlong>(&((cdts*)(pcdts))[index]);
	}
	catch(std::exception& err)
	{
		throwMetaFFIException(env, err.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jobject JNICALL Java_metaffi_MetaFFIBridge_get_1object(JNIEnv* env, jclass, jlong phandle)
{
	if(openjdk_objects_table::instance().contains((jobject)phandle)){
		throwMetaFFIException(env, "Object is not found in Objects Table");
	}
	
	return (jobject)phandle;
}
//--------------------------------------------------------------------
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_remove_1object (JNIEnv* env, jclass, jlong phandle)
{
	openjdk_objects_table::instance().remove(env, (jobject)phandle);
}
//--------------------------------------------------------------------
int get_array_dimensions(JNIEnv *env, jobjectArray arr)
{
	jclass cls = env->GetObjectClass(arr);
	jmethodID mid = env->GetMethodID(cls, "getClass", "()Ljava/lang/Class;");
	jobject clsObj = env->CallObjectMethod(arr, mid);
	cls = env->GetObjectClass(clsObj);
	mid = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
	jstring name = (jstring)env->CallObjectMethod(clsObj, mid);
	const char* nameStr = env->GetStringUTFChars(name, nullptr);
	
	int dimensions = 0;
	for (const char* c = nameStr; *c != '\0'; c++)
	{
		if (*c == '[') {
			dimensions++;
		}
	}
	
	env->ReleaseStringUTFChars(name, nameStr);
	
	return dimensions;
}
//--------------------------------------------------------------------
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_java_1to_1cdts(JNIEnv* env, jclass, jlong pcdts, jobjectArray parameters, jlongArray types)
{
	try
	{
		jsize l = env->GetArrayLength(parameters);
		cdts_java_wrapper wrapper((cdts*)pcdts);

		jlong* types_elements = env->GetLongArrayElements(types, nullptr);
		check_and_throw_jvm_exception(env, types_elements);
		
		metaffi::utils::scope_guard sg([&](){ env->ReleaseLongArrayElements(types, types_elements, 0); });
		
		for(jsize i=0 ; i < l ; i++)
		{
			jvalue cur_object;
			cur_object.l = env->GetObjectArrayElement(parameters, i);
			check_and_throw_jvm_exception(env, true);
			metaffi_type_info type_to_expect = (types_elements[i] & metaffi_array_type) == 0 ?
												(types_elements[i] == metaffi_callable_type ? metaffi_type_info{metaffi_callable_type} : metaffi_type_info{(metaffi_type)types_elements[i]}) :
                                               metaffi_type_info{(uint64_t)types_elements[i], nullptr, false, get_array_dimensions(env, (jobjectArray)cur_object.l)};
			wrapper.from_jvalue(env, cur_object, 'L', type_to_expect, i);
			wrapper.switch_to_primitive(env, i, types_elements[i]);
		}
		
		return (jlong)(cdts*)wrapper;
	}
	catch(std::exception& exp)
	{
		throwMetaFFIException(env, exp.what());
		return 0;
	}
}
//--------------------------------------------------------------------
JNIEXPORT jobjectArray JNICALL Java_metaffi_MetaFFIBridge_cdts_1to_1java(JNIEnv* env, jclass, jlong pcdts, jlong length)
{
	try
	{
		cdts_java_wrapper wrapper((cdts*)pcdts);
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
