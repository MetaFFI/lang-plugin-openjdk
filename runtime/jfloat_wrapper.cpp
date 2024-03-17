#include "jfloat_wrapper.h"
#include <string>

jfloat_wrapper::jfloat_wrapper(jfloat value) : value(value)
{}

jfloat_wrapper::jfloat_wrapper(const jfloat_wrapper& other) : value(other.value)
{}

jfloat_wrapper::operator jfloat()
{
	return value;
}

jfloat_wrapper& jfloat_wrapper::operator=(jfloat val)
{
	value = val;
	return *this;
}

jfloatArray jfloat_wrapper::new_1d_array(JNIEnv* env, jsize size, const jfloat* parr)
{
	if(size < 0)
	{
		throw std::runtime_error("Size cannot be negative");
	}
	
	jfloatArray array = env->NewFloatArray(size);
	if(!array)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	if(parr != nullptr)
	{
		env->SetFloatArrayRegion(array, 0, size, parr);
	}
	
	return array;
}