#include <sstream>
#include "classes_repository.h"

std::unique_ptr<classes_repository> classes_repository::instance;


//--------------------------------------------------------------------
classes_repository& classes_repository::get_instance()
{
	// IMPORTANT! This singleton is not thread-safe! If this needs to be thread-safe, use std::once.
	
	if(!classes_repository::instance)
	{
		classes_repository::instance = std::make_unique<classes_repository>();
	}
	
	return *classes_repository::instance;
}
//--------------------------------------------------------------------
void classes_repository::free_instance()
{
	classes_repository::instance = nullptr;
}
//--------------------------------------------------------------------
void classes_repository::init(std::shared_ptr<jvm> pjvm)
{
	this->pjvm = pjvm;
}
//--------------------------------------------------------------------
jclass classes_repository::add(const std::string& module_name)
{
	auto it = this->classes.find(module_name);
	if(it != this->classes.end()){
		return it->second;
	}
	
	// load the JAR and class created by OpenFFI and their functions
	// [module/path]:[Classname]
	std::string::size_type idx = module_name.find(':');
	if(idx == std::string::npos)
	{
		std::stringstream ss;
		ss << module_name << " is illegal module name";
		throw std::runtime_error(ss.str());
	}
	
	std::string module = module_name.substr(0, idx);
	std::string classname = module_name.substr(idx+1, module_name.size()-(idx-1));
	
	jclass cls = pjvm->load_class(module, classname);
	
	this->classes[module_name] = cls;
	
	return cls;
}
//--------------------------------------------------------------------
void classes_repository::remove(const std::string &module_name)
{
	auto it = this->classes.find(module_name);
	if(it != this->classes.end())
	{
		pjvm->free_class(it->second);
		this->classes.erase(it);
	}
}
//--------------------------------------------------------------------
jclass classes_repository::get(const std::string &module_name)
{
	auto it = this->classes.find(module_name);
	if(it == this->classes.end())
	{
		std::stringstream ss;
		ss << module_name << " not found in classes repository";
		throw std::runtime_error(ss.str());
	}
	
	return it->second;
}
//--------------------------------------------------------------------