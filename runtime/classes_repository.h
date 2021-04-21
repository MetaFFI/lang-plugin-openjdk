#pragma once
#include <memory>
#include <unordered_map>
#include <boost/dll.hpp>
#include <jni.h>
#include <jvm.h>

//--------------------------------------------------------------------
class classes_repository
{
private: // variable
	static std::unique_ptr<classes_repository> instance;

private: // methods
	std::unordered_map<std::string, jclass> classes;
	std::shared_ptr<jvm> pjvm;

public: // static functions
	static classes_repository& get_instance();
	static void free_instance();

public: // methods
	classes_repository() = default;
	~classes_repository() = default;
	
	void init(std::shared_ptr<jvm> pjvm);
	
	jclass get(const std::string& module_name, const std::string& class_name, bool load_if_missing);
	jclass add(const std::string& module_name, const std::string& class_name);
	void remove(const std::string& module_name, const std::string& class_name);
};
//--------------------------------------------------------------------