#include "jshort_wrapper.h"
#include <string>

jshort_wrapper::jshort_wrapper(jshort value) : value(value)
{}

jshort_wrapper::jshort_wrapper(const jshort_wrapper& other) : value(other.value)
{}

jshort_wrapper::operator jshort()
{
	return value;
}

jshort_wrapper& jshort_wrapper::operator=(jshort val)
{
	value = val;
	return *this;
}

jshortArray jshort_wrapper::new_1d_array(JNIEnv* env, jsize size, const jshort* parr)
{
	if(size < 0)
	{
		throw std::runtime_error("Size cannot be negative");
	}
	
	jshortArray array = env->NewShortArray(size);
	if(!array)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	if(parr != nullptr)
	{
		env->SetShortArrayRegion(array, 0, size, parr);
	}
	
	return array;
}