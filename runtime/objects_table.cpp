#include "objects_table.h"
#include <mutex>

//--------------------------------------------------------------------
void openjdk_objects_table_impl::free()
{
	std::unique_lock l(m);
	
	// TODO: Reference count handling

}
//--------------------------------------------------------------------
void openjdk_objects_table_impl::set(jobject obj)
{
	std::unique_lock l(m); // TODO need to use upgradable lock!
	
	auto it = this->objects.find(obj);
	if(it != this->objects.end()){
		return;
	}

	this->objects.insert(obj);
}
//--------------------------------------------------------------------
void openjdk_objects_table_impl::remove(JNIEnv* env, jobject obj)
{
	std::unique_lock l(m);
	
	auto it = this->objects.find(obj);
	if(it == this->objects.end()){
		return;
	}

	if(env->GetObjectRefType(obj) == JNIGlobalRefType)
	{
		env->DeleteGlobalRef(obj);
	}

	this->objects.erase(obj);
}
//--------------------------------------------------------------------
bool openjdk_objects_table_impl::contains(jobject obj) const
{
	std::shared_lock l(m);
	return this->objects.find(obj) != this->objects.end();
}
//--------------------------------------------------------------------
size_t openjdk_objects_table_impl::size() const
{
	std::shared_lock l(m);
	return this->objects.size();
}
//--------------------------------------------------------------------
void openjdk_release_object(JNIEnv* env, metaffi_handle h)
{
	openjdk_objects_table::instance().remove(env, (jobject)h);
}
//--------------------------------------------------------------------