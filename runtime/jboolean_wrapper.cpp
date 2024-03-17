#include "jboolean_wrapper.h"
#include <string>

jboolean_wrapper::jboolean_wrapper(jboolean value) : value(value)
{}

jboolean_wrapper::jboolean_wrapper(const jboolean_wrapper& other) : value(other.value)
{}

jboolean_wrapper::operator jboolean()
{
	return value;
}

jboolean_wrapper& jboolean_wrapper::operator=(jboolean val)
{
	value = val;
	return *this;
}

jbooleanArray jboolean_wrapper::new_1d_array(JNIEnv* env, jsize size, const jboolean* parr)
{
	if(size < 0)
	{
		throw std::runtime_error("Size cannot be negative");
	}
	
	jbooleanArray array = env->NewBooleanArray(size);
	if(!array)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	if(parr != nullptr)
	{
		env->SetBooleanArrayRegion(array, 0, size, parr);
	}
	
	return array;
}