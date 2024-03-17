#include <jni.h>
#include <string>
#include "runtime/metaffi_primitives.h"
#include <vector>

class jchar_wrapper
{
private:
	JNIEnv* env;
	jchar value;

public:
	jchar_wrapper(JNIEnv* env, char8_t c);
	jchar_wrapper(JNIEnv* env, char16_t c);
	jchar_wrapper(JNIEnv* env, char32_t c);
	
	jchar_wrapper(JNIEnv* env, jchar c) : env(env), value(c){}
	
	explicit operator jchar() const;
	explicit operator std::u8string() const;
	explicit operator char16_t() const;
	explicit operator char32_t() const;
	
	static jcharArray new_1d_array(JNIEnv* env, const char8_t* s, metaffi_size length);
	static jcharArray new_1d_array(JNIEnv* env, const char16_t* s, metaffi_size length);
	static jcharArray new_1d_array(JNIEnv* env, const char32_t* s, metaffi_size length);
	
	template<typename T>
	static T* to_c_array(JNIEnv* env, jarray jarr);
};

template<>
inline char8_t* jchar_wrapper::to_c_array<char8_t>(JNIEnv* env, jarray jarr)
{
	jsize length = env->GetArrayLength(jarr);
	std::vector<char8_t> c_vector;
	jchar* elements = env->GetCharArrayElements((jcharArray)jarr, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jchar_wrapper wrapper(env, elements[i]);
		std::u8string utf8 = static_cast<std::u8string>(wrapper);
		c_vector.insert(c_vector.end(), utf8.begin(), utf8.end());
	}
	env->ReleaseCharArrayElements((jcharArray)jarr, elements, 0);
	
	char8_t* c_array = new char8_t[c_vector.size()];
	std::copy(c_vector.begin(), c_vector.end(), c_array);
	return c_array;
}

template<>
inline char16_t* jchar_wrapper::to_c_array<char16_t>(JNIEnv* env, jarray jarr)
{
	jsize length = env->GetArrayLength(jarr);
	char16_t* c_array = new char16_t[length];
	jchar* elements = env->GetCharArrayElements((jcharArray)jarr, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jchar_wrapper wrapper(env, elements[i]);
		c_array[i] = static_cast<char16_t>(wrapper);
	}
	env->ReleaseCharArrayElements((jcharArray)jarr, elements, 0);
	return c_array;
}

template<>
inline char32_t* jchar_wrapper::to_c_array<char32_t>(JNIEnv* env, jarray jarr)
{
	jsize length = env->GetArrayLength(jarr);
	char32_t* c_array = new char32_t[length];
	jchar* elements = env->GetCharArrayElements((jcharArray)jarr, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jchar_wrapper wrapper(env, elements[i]);
		c_array[i] = static_cast<char32_t>(wrapper);
	}
	env->ReleaseCharArrayElements((jcharArray)jarr, elements, 0);
	return c_array;
}