#ifdef _MSC_VER
#include <corecrt.h> // https://www.reddit.com/r/cpp_questions/comments/qpo93t/error_c2039_invalid_parameter_is_not_a_member_of/
#endif
#pragma once
#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

#include <vector>
#include <string>
#include <functional>
#include <runtime/cdt_structs.h>
#include <stdexcept>

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

	static std::string get_exception_description(JNIEnv* env, jthrowable throwable);
	std::string get_exception_description(jthrowable throwable);
	
	explicit operator JavaVM*();
	
private:
	static void check_throw_error(jint err);
	
};
