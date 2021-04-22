#pragma once
#include <jni.h>
#include <vector>
#include <string>

#define check_and_throw_jvm_exception(jvm_instance, env) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = jvm_instance->get_exception_description(env->ExceptionOccurred());\
throw std::runtime_error(err_msg);\
}

class jvm
{
private:
	JavaVM* pjvm = nullptr;
	JNIEnv* penv = nullptr;
	jclass ObjectLoader_class;
	jmethodID loadObject_method;

public:
	jvm(const std::string& classpath);
	~jvm() = default;
	
	void fini();
	
	jclass load_class(const std::string& dir_or_jar, const std::string& class_name);
	void free_class(jclass obj);
	
	explicit operator JNIEnv*(){ return penv; }
	
	std::string get_exception_description(jthrowable throwable);

private:
	static void check_throw_error(jint err);
};
