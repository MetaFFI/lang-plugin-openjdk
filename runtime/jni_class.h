#pragma once
#include "jvm.h"

#include "argument_definition.h"
#include <vector>
#include <memory>
#include <runtime/cdts_wrapper.h>
#include "cdts_java_wrapper.h"


class jni_class
{
private:
	JNIEnv* env;
	jclass cls;
	std::shared_ptr<jvm> pjvm;

public:
	jni_class(std::shared_ptr<jvm> pjvm, JNIEnv* env, jclass cls);
	~jni_class() = default;
	
	jmethodID load_method(const std::string& method_name, const argument_definition& return_type, const std::vector<argument_definition>& parameters_types, bool is_static);
	jfieldID load_field(const std::string& field_name, const argument_definition& field_type, bool is_static);
	
	void write_field_to_cdts(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id, metaffi_type t);
	void write_cdts_to_field(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id);
	
	void call(const cdts_java_wrapper& params_wrapper, const cdts_java_wrapper& retval_wrapper, metaffi_type retval_type, bool instance_required, bool is_constructor, const std::set<uint8_t>& any_type_indices, jmethodID method);
	
	void validate_valid_jobject(jobject obj);
	
	operator jclass(){ return cls; }
	
	std::string get_method_name(JNIEnv *env, jclass cls, jmethodID mid) const;
	
};

