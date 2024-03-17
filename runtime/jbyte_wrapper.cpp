#include "jbyte_wrapper.h"
#include <string>
#include <stdexcept>
#include "exception_macro.h"

jbyte_wrapper::jbyte_wrapper(jbyte value) : value(value)
{}

jbyte_wrapper::jbyte_wrapper(const jbyte_wrapper& other) : value(other.value)
{}

jbyte_wrapper::operator jbyte() const
{
	return value;
}

jbyte_wrapper::operator metaffi_uint8() const
{
	return static_cast<metaffi_uint8>(value);
}

jbyte_wrapper& jbyte_wrapper::operator=(jbyte val)
{
	value = val;
	return *this;
}

jbyteArray jbyte_wrapper::new_1d_array(JNIEnv* env, jsize size, const jbyte* parr /*= nullptr*/)
{
	if(size < 0)
	{
		throw std::runtime_error("Size cannot be negative");
	}
	
	jbyteArray array = env->NewByteArray(size);
	if(!array)
	{
		check_and_throw_jvm_exception(env, true);
	}
	
	if(parr != nullptr)
	{
		env->SetByteArrayRegion(array, 0, size, parr);
	}
	
	return array;
}
