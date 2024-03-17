#include "jchar_wrapper.h"
#include "runtime/metaffi_primitives.h"

jchar_wrapper::jchar_wrapper(JNIEnv* env, char8_t c) : env(env)
{
	value = static_cast<jchar>(c);
}

jchar_wrapper::jchar_wrapper(JNIEnv* env, char16_t c) : env(env)
{
	value = static_cast<jchar>(c);
}

jchar_wrapper::jchar_wrapper(JNIEnv* env, char32_t c) : env(env)
{
	std::u16string u16(1, static_cast<char16_t>(c));
	value = static_cast<jchar>(u16[0]);
}

jchar_wrapper::operator jchar() const
{
	return value;
}

jchar_wrapper::operator std::u8string() const
{
	char utf8[4] = {};
	std::mbstate_t state = std::mbstate_t();
	c16rtomb(utf8, value, &state);
	return reinterpret_cast<char8_t*>(utf8);
}

jchar_wrapper::operator char16_t() const
{
	return static_cast<char16_t>(value);
}

jchar_wrapper::operator char32_t() const
{
	char32_t utf32;
	std::mbstate_t state = std::mbstate_t();
	const char16_t* from_next;
	char32_t* to_next;
	c16rtomb(reinterpret_cast<char*>(&utf32), value, &state);
	return utf32;
}

jcharArray jchar_wrapper::new_1d_array(JNIEnv* env, const char8_t* s, metaffi_size length)
{
	jcharArray array = env->NewCharArray(length);
	jchar* elements = env->GetCharArrayElements(array, nullptr);
	for(size_t i = 0; i < std::char_traits<char8_t>::length(s); ++i)
	{
		elements[i] = static_cast<jchar>(s[i]);
	}
	env->ReleaseCharArrayElements(array, elements, 0);
	return array;
}

jcharArray jchar_wrapper::new_1d_array(JNIEnv* env, const char16_t* s, metaffi_size length)
{
	jcharArray array = env->NewCharArray(length);
	jchar* elements = env->GetCharArrayElements(array, nullptr);
	for(size_t i = 0; i < std::char_traits<char16_t>::length(s); ++i)
	{
		elements[i] = static_cast<jchar>(s[i]);
	}
	env->ReleaseCharArrayElements(array, elements, 0);
	return array;
}

jcharArray jchar_wrapper::new_1d_array(JNIEnv* env, const char32_t* s, metaffi_size length)
{
	std::u16string u16(reinterpret_cast<const char16_t*>(s), length);
	jcharArray array = env->NewCharArray(u16.length());
	jchar* elements = env->GetCharArrayElements(array, nullptr);
	for(size_t i = 0; i < u16.length(); ++i)
	{
		elements[i] = static_cast<jchar>(u16[i]);
	}
	env->ReleaseCharArrayElements(array, elements, 0);
	return array;
}