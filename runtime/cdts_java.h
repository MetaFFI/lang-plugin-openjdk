#pragma once
#include "runtime/cdts_wrapper.h"
#include <unordered_map>
#include <memory>
#include <jni.h>
#include <vector>
#include <jvm.h>

class cdts_java
{
private:
	metaffi::runtime::cdts_wrapper cdts;
	JNIEnv* env = nullptr;
	
public:
	explicit cdts_java(cdt* cdts, metaffi_size cdts_length, JNIEnv* env);
	
	cdt* get_cdts();
	
	/**
	 * @brief Parses CDTS and returns Object[]. Method assumes CDTS is filled!
	 */
	jobjectArray parse();
	
	void build(jobjectArray parameters, metaffi_types_ptr parameters_types, int params_count, int starting_index);
	
	static jclass float_class;
	static jmethodID float_constructor;
	static jmethodID float_get_value;
	static jclass float_array_class;
	
	static jclass double_class;
	static jmethodID double_constructor;
	static jmethodID double_get_value;
	static jclass double_array_class;
	
	static jclass byte_class;
	static jmethodID byte_constructor;
	static jmethodID byte_get_value;
	static jclass byte_array_class;
	
	static jclass short_class;
	static jmethodID short_constructor;
	static jmethodID short_get_value;
	static jclass short_array_class;
	
	static jclass int_class;
	static jmethodID int_constructor;
	static jmethodID int_get_value;
	static jclass int_array_class;
	
	static jclass long_class;
	static jmethodID long_constructor;
	static jmethodID long_get_value;
	static jclass long_array_class;
	
	static jclass boolean_class;
	static jmethodID boolean_constructor;
	static jmethodID boolean_get_value;
	static jclass boolean_array_class;
	
	static jclass char_class;
	static jmethodID char_constructor;
	static jmethodID char_get_value;
	static jclass char_array_class;
	
	static jclass string_class;
	static jmethodID string_constructor;
	static jclass string_array_class;
	
	static jclass object_class;
	static jclass object_array_class;
	
	static jclass metaffi_handle_class;
	static jmethodID metaffi_handle_constructor;
	static jmethodID metaffi_handle_get_value;
	
private:
	static std::unordered_map<std::string, metaffi_types> java_to_metaffi_types;
	
	
	
	
};
