#include "jobjectArray_wrapper.h"
#include <string>

jobjectArray_wrapper::jobjectArray_wrapper(JNIEnv* env, jobjectArray array) : env(env), array(array)
{}

int jobjectArray_wrapper::size()
{
	jsize l = env->GetArrayLength(array);
	check_and_throw_jvm_exception(env, true);
	return l;
}

jobject jobjectArray_wrapper::get(int index)
{
	jobject o = env->GetObjectArrayElement(array, index);
	if(!o)
	{
		check_and_throw_jvm_exception(env, true);
	}
	return o;
}

void jobjectArray_wrapper::set(int index, jobject obj)
{
	env->SetObjectArrayElement(array, index, obj);
	check_and_throw_jvm_exception(env, true);
}

jobjectArray jobjectArray_wrapper::create_array(JNIEnv* env, const char* class_name, int size, int dimensions)
{
	std::string class_str(dimensions-1, '[');
	class_str += class_name;
	
	jclass cls = env->FindClass(class_str.c_str());
	if(!cls)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	jobjectArray arr = env->NewObjectArray(size, cls, nullptr);
	if(!arr)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	return arr;
}

jobjectArray_wrapper::operator jobject()
{
	return static_cast<jobject>(array);
}

jobjectArray_wrapper::operator jobjectArray()
{
	return array;
}