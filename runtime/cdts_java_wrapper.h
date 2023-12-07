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
	
	jvalue to_jvalue(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	void from_jvalue(std::shared_ptr<jvm> pjvm, JNIEnv* env, jvalue val, metaffi_type type, int index) const;
	
	void switch_to_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int i) const; // switch the type inside to java object (if possible)
	void switch_to_primitive(std::shared_ptr<jvm> pjvm, JNIEnv* env, int i) const; // switch the type inside to integer (if possible)
	
	char get_jni_signature(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	
	bool is_jstring(JNIEnv* env, int index) const;
	
	
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, metaffi_int32 val) const;
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, bool val) const;
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, metaffi_int8 val) const;
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, metaffi_uint16 val) const;
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, metaffi_int16 val) const;
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, metaffi_int64 val) const;
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, metaffi_float32 val) const;
	void set_object(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index, metaffi_float64 val) const;
	
	metaffi_int32 get_object_as_int32(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	bool get_object_as_bool(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	metaffi_int8 get_object_as_int8(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	metaffi_uint16 get_object_as_uint16(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	metaffi_int16 get_object_as_int16(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	metaffi_int64 get_object_as_int64(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	metaffi_float32 get_object_as_float32(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	metaffi_float64 get_object_as_float64(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	std::string get_object_as_string(std::shared_ptr<jvm> pjvm, JNIEnv* env, int index) const;
	
	bool is_jobject(int i) const;
};
