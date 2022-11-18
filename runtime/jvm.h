#pragma once
#include <jni.h>
#include <vector>
#include <string>
#include <functional>
#include <runtime/cdt_structs.h>

#define check_and_throw_jvm_exception(jvm_instance, env, var) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = jvm_instance->get_exception_description(env->ExceptionOccurred());\
throw std::runtime_error(err_msg);\
}\
else if(!var)\
{\
throw std::runtime_error("Failed to get " #var);\
}

class jvm
{
private:
	JavaVM* pjvm = nullptr;
	bool is_destroy = false;

public:
	jvm();
	~jvm() = default;
	
	void fini();
	
	void load_function_path(const std::string& function_path, jclass* cls, jmethodID* meth);
	
	// returns release environment function
	// TODO: add scoped wrapper
	std::function<void()> get_environment(JNIEnv** env);
	
	std::string get_exception_description(jthrowable throwable);
	
	explicit operator JavaVM*();
	
private:
	static void check_throw_error(jint err);
	
};
