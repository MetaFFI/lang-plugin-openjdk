#pragma once
#include "utils/singleton.hpp"
#include <set>
#include "runtime/metaffi_primitives.h"
#include <shared_mutex>
#include <jni.h>


extern "C" void release_object(metaffi_handle h);

class objects_table_impl
{
private:
	std::set<jobject> objects;
	mutable std::shared_mutex m;

public:
	objects_table_impl() = default;
	~objects_table_impl() = default;
	
	void free();
	
	void set(jobject obj);
	void remove(jobject obj);
	bool contains(jobject obj) const;
	
	size_t size() const;
};

typedef metaffi::utils::singleton<objects_table_impl> objects_table;
template class metaffi::utils::singleton<objects_table_impl>;