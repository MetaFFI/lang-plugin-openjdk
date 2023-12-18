#pragma once
#include "utils/singleton.hpp"
#include <set>
#include "runtime/metaffi_primitives.h"
#include <shared_mutex>
#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif


extern "C" void openjdk_release_object(metaffi_handle h);

class openjdk_objects_table_impl
{
private:
	std::set<jobject> objects;
	mutable std::shared_mutex m;

public:
	openjdk_objects_table_impl() = default;
	~openjdk_objects_table_impl() = default;
	
	void free();
	
	void set(jobject obj);
	void remove(JNIEnv* env, jobject obj);
	bool contains(jobject obj) const;
	
	size_t size() const;
};

typedef metaffi::utils::singleton<openjdk_objects_table_impl> openjdk_objects_table;
template class metaffi::utils::singleton<openjdk_objects_table_impl>;