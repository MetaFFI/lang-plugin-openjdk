#pragma once
#include "utils/singleton.hpp"
#include <set>
#include "runtime/metaffi_primitives.h"
#include <shared_mutex>
#include <jni.h>


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
	void remove(jobject obj);
	bool contains(jobject obj) const;
	
	size_t size() const;
};

typedef metaffi::utils::singleton<openjdk_objects_table_impl> openjdk_objects_table;
template class metaffi::utils::singleton<openjdk_objects_table_impl>;