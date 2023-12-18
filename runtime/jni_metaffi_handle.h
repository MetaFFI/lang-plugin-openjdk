#include "jvm.h"
#include <memory>

class jni_metaffi_handle
{
private:
	std::shared_ptr<jvm> pjvm;
	metaffi_handle handle = nullptr;
	uint64_t runtime_id{};

	static jclass metaffi_handle_class;
	static jmethodID get_handle_id;
	static jmethodID get_runtime_id_id;
	static jmethodID metaffi_handle_constructor;
	
	jni_metaffi_handle(JNIEnv* env);
	
public:
	static bool is_metaffi_handle_wrapper_object(JNIEnv* env, jobject o);
	
	jni_metaffi_handle(JNIEnv* env, metaffi_handle v, uint64_t runtime_id);
	jni_metaffi_handle(JNIEnv* env, jobject obj);
	
	metaffi_handle get_handle();
	uint64_t get_runtime_id();
	
	jobject new_jvm_object(JNIEnv* env);
};