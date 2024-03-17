#pragma once
#include "jvm.h"

#include "argument_definition.h"
#include <vector>
#include <memory>
#include <runtime/cdts_wrapper.h>
#include <set>

class cdts_java_wrapper : public metaffi::runtime::cdts_wrapper
{
public:
	explicit cdts_java_wrapper(cdt* cdts, metaffi_size cdts_length);
	
	jvalue to_jvalue(JNIEnv* env, int index) const;
	void from_jvalue(JNIEnv* env, jvalue val, const metaffi_type_info&, int index) const;
	
	void switch_to_object(JNIEnv* env, int i) const; // switch the type inside to java object (if possible)
	void switch_to_primitive(JNIEnv* env, int i, metaffi_type t = metaffi_any_type) const; // switch the type inside to integer (if possible)
	
	char get_jni_primitive_signature_from_object_form_of_primitive(JNIEnv* env, int index) const;
	
	bool is_jstring(JNIEnv* env, int index) const;
	
	
	void set_object(JNIEnv* env, int index, metaffi_int32 val) const;
	void set_object(JNIEnv* env, int index, bool val) const;
	void set_object(JNIEnv* env, int index, metaffi_int8 val) const;
	void set_object(JNIEnv* env, int index, metaffi_uint16 val) const;
	void set_object(JNIEnv* env, int index, metaffi_int16 val) const;
	void set_object(JNIEnv* env, int index, metaffi_int64 val) const;
	void set_object(JNIEnv* env, int index, metaffi_float32 val) const;
	void set_object(JNIEnv* env, int index, metaffi_float64 val) const;
	
	bool is_jobject(int i) const;
};
