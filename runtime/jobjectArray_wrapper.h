#pragma once

#include <jni.h>
#include "exception_macro.h"

class jobjectArray_wrapper
{
private:
	JNIEnv* env;
	jobjectArray array;
	
public:
	static jobjectArray create_array(JNIEnv* env, const char* class_name, int size, int dimensions);

public:
	explicit jobjectArray_wrapper(JNIEnv* env, jobjectArray array);
	
	int size();
	
	jobject get(int index);
	void set(int index, jobject obj);
	
	explicit operator jobject();
	explicit operator jobjectArray();
};