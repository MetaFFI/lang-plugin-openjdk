#include "jstring_wrapper.h"


jstring_wrapper::jstring_wrapper(JNIEnv* env, const char* s) : env(env)
{
	value = env->NewStringUTF(s);
}

jstring_wrapper::jstring_wrapper(JNIEnv* env, const char16_t* s) : env(env)
{
	value = env->NewString(reinterpret_cast<const jchar*>(s), std::char_traits<char16_t>::length(s));
}

jstring_wrapper::jstring_wrapper(JNIEnv* env, const char32_t* s) : env(env)
{
	std::u16string temp(s, s + std::char_traits<char32_t>::length(s));
	value = env->NewString(reinterpret_cast<const jchar*>(temp.c_str()), temp.length());
}

jstring_wrapper::jstring_wrapper(JNIEnv* env, jstring s) : env(env), value(s) {}

jobjectArray jstring_wrapper::new_1d_array(JNIEnv* env, const metaffi_string8* s, jsize length)
{
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(length, stringClass, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jstring_wrapper wrapper(env, reinterpret_cast<const char*>(s[i]));
		env->SetObjectArrayElement(array, i, (jstring)wrapper);
	}
	return array;
}

jobjectArray jstring_wrapper::new_1d_array(JNIEnv* env, const metaffi_string16* s, jsize length)
{
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(length, stringClass, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jstring_wrapper wrapper(env, s[i]);
		env->SetObjectArrayElement(array, i, (jstring)wrapper);
	}
	return array;
}

jobjectArray jstring_wrapper::new_1d_array(JNIEnv* env, const metaffi_string32* s, jsize length)
{
	jclass stringClass = env->FindClass("java/lang/String");
	jobjectArray array = env->NewObjectArray(length, stringClass, nullptr);
	for(jsize i = 0; i < length; ++i)
	{
		jstring_wrapper wrapper(env, s[i]);
		env->SetObjectArrayElement(array, i, (jstring)wrapper);
	}
	return array;
}

jstring_wrapper::operator jstring()
{
	return value;
}

jstring_wrapper::operator const char8_t*()
{
	const char* original = env->GetStringUTFChars(value, nullptr);
	std::string temp(original);
	env->ReleaseStringUTFChars(value, original);
	char8_t* copy = new char8_t[temp.length() + 1];
	std::copy(temp.begin(), temp.end(), copy);
	copy[temp.length()] = '\0'; // null terminate the string
	return copy;
}

jstring_wrapper::operator const char16_t*()
{
	const jchar* temp = env->GetStringChars(value, nullptr);
	char16_t* utf16String = (char16_t*)malloc((std::char_traits<char16_t>::length(reinterpret_cast<const char16_t*>(temp)) + 1) * sizeof(char16_t));
	std::copy(temp, temp + std::char_traits<char16_t>::length(reinterpret_cast<const char16_t*>(temp)), utf16String);
	env->ReleaseStringChars(value, temp);
	return utf16String;
}

jstring_wrapper::operator const char32_t*()
{
	const jchar* temp = env->GetStringChars(value, nullptr);
	std::u16string u16(reinterpret_cast<const char16_t*>(temp), env->GetStringLength(value));
	char32_t* utf32String = (char32_t*)malloc((u16.length() + 1) * sizeof(char32_t));
	std::copy(u16.begin(), u16.end(), utf32String);
	utf32String[u16.length()] = '\0';
	env->ReleaseStringChars(value, temp);
	return utf32String;
}