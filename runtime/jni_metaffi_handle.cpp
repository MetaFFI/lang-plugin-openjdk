#include "jni_metaffi_handle.h"

#include <utility>
#include "jni_class.h"
#include "class_loader.h"
#include "exception_macro.h"


jclass jni_metaffi_handle::metaffi_handle_class = nullptr;
jmethodID jni_metaffi_handle::get_handle_id = nullptr;
jmethodID jni_metaffi_handle::get_runtime_id_id = nullptr;
jmethodID jni_metaffi_handle::metaffi_handle_constructor = nullptr;



//--------------------------------------------------------------------
jni_metaffi_handle::jni_metaffi_handle(JNIEnv* env)
{
	if(!metaffi_handle_class)
	{
		std::string openjdk_bridge_url = (std::string("file://") + std::getenv("METAFFI_HOME")) + "/xllr.openjdk.bridge.jar";
		jni_class_loader clsloader(env, openjdk_bridge_url);
		metaffi_handle_class = (jclass)clsloader.load_class("metaffi/MetaFFIHandle");
	}
	
	if(!get_handle_id)
	{
		get_handle_id = env->GetMethodID(metaffi_handle_class, "Handle", "()J");
		check_and_throw_jvm_exception(env, true);
	}
	
	if(!get_runtime_id_id)
	{
		get_runtime_id_id = env->GetMethodID(metaffi_handle_class, "RuntimeID", "()J");
		check_and_throw_jvm_exception(env, true);
	}
	
	if(!metaffi_handle_constructor)
	{
		metaffi_handle_constructor = env->GetMethodID(metaffi_handle_class, "<init>", "(JJ)V");
		check_and_throw_jvm_exception(env, true);
	}
}
//--------------------------------------------------------------------
jni_metaffi_handle::jni_metaffi_handle(JNIEnv* env, metaffi_handle v, uint64_t runtime_id):jni_metaffi_handle(env)
{
	this->handle = v;
	this->runtime_id = runtime_id;
}
//--------------------------------------------------------------------
jni_metaffi_handle::jni_metaffi_handle(JNIEnv* env, jobject obj):jni_metaffi_handle(env)
{
	this->handle = (void*)env->CallLongMethod(obj, get_handle_id);
	check_and_throw_jvm_exception(env, true);
	
	this->runtime_id = (uint64_t)env->CallLongMethod(obj, get_runtime_id_id);
	check_and_throw_jvm_exception(env, true);
}
//--------------------------------------------------------------------
jobject jni_metaffi_handle::new_jvm_object(JNIEnv* env)
{
	jobject res = env->NewObject(metaffi_handle_class, metaffi_handle_constructor, (jlong)this->handle, (jlong)this->runtime_id);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
bool jni_metaffi_handle::is_metaffi_handle_wrapper_object(JNIEnv* env, jobject o)
{
	// if(!metaffi_handle_class)
	{
		jni_class_loader clsloader(env, "");
		metaffi_handle_class = (jclass)clsloader.load_class("metaffi/MetaFFIHandle");
		metaffi_handle_class = (jclass)env->NewGlobalRef(metaffi_handle_class);
	}

	// if(!get_handle_id)
	{
		get_handle_id = env->GetMethodID(metaffi_handle_class, "Handle", "()J");
		check_and_throw_jvm_exception(env, true);
	}

	// if(!get_runtime_id_id)
	{
		get_runtime_id_id = env->GetMethodID(metaffi_handle_class, "RuntimeID", "()J");
		check_and_throw_jvm_exception(env, true);
	}

	// if(!metaffi_handle_constructor)
	{
		metaffi_handle_constructor = env->GetMethodID(metaffi_handle_class, "<init>", "(JJ)V");
		check_and_throw_jvm_exception(env, true);
	};
	
	return env->IsInstanceOf(o, metaffi_handle_class) != JNI_FALSE;
}
//--------------------------------------------------------------------
metaffi_handle jni_metaffi_handle::get_handle()
{
	return this->handle;
}
//--------------------------------------------------------------------
uint64_t jni_metaffi_handle::get_runtime_id()
{
	return this->runtime_id;
}
//--------------------------------------------------------------------
