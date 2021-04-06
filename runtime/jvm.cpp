#include "jvm.h"
#include <stdexcept>
#include <sstream>
#include <utils/scope_guard.hpp>

using namespace openffi::utils;

//--------------------------------------------------------------------
jvm::jvm()
{
	// if there's a JVM already loaded, get it.
	jsize nVMs;
	check_throw_error(JNI_GetCreatedJavaVMs(nullptr, 0, &nVMs));
	
	if(nVMs > 0)
	{
		check_throw_error(JNI_GetCreatedJavaVMs(&this->pjvm, 1, &nVMs));
		return;
	}
	
	std::stringstream ss;
	ss << "-Djava.class.path=" << std::getenv("OPENFFI_HOME") << "/xllr_java_bridge.jar"; // TODO: do not load this in options to support already running JVM
	std::string options_string = ss.str();
	JavaVMOption options[1] = {0};
	options[0].optionString = (char*)options_string.c_str();
	
	// set initialization args
	JavaVMInitArgs vm_args = {0};
	vm_args.version = JNI_VERSION_10;
	vm_args.nOptions = 1;
	vm_args.options = &options[0];
	vm_args.ignoreUnrecognized = JNI_FALSE;
	
	// load jvm
	check_throw_error(JNI_CreateJavaVM(&this->pjvm, (void**)&penv, &vm_args));
	
	// load ObjectLoader
	this->ObjectLoader_class = penv->FindClass("openffi/ObjectLoader");
	check_and_throw_jvm_exception(this, penv);
	
	this->loadObject_method = penv->GetStaticMethodID(ObjectLoader_class, "loadObject", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");
	check_and_throw_jvm_exception(this, penv)
}
//--------------------------------------------------------------------
void jvm::fini()
{
	if(this->pjvm)
	{
		this->pjvm->DestroyJavaVM();
		this->pjvm = nullptr;
	}
}
//--------------------------------------------------------------------
void jvm::check_throw_error(jint err)
{
	if(err == JNI_OK){
		return;
	}
	
	switch (err)
	{
		case JNI_ERR:
			throw std::runtime_error("Unknown error has occurred");
		
		case JNI_EDETACHED:
			throw std::runtime_error("Thread detached from the JVM");
		
		case JNI_EVERSION:
			throw std::runtime_error("JNI version error");
		
		case JNI_ENOMEM:
			throw std::runtime_error("Not enough memory");
		
		case JNI_EEXIST:
			throw std::runtime_error("JVM already created");
		
		case JNI_EINVAL:
			throw std::runtime_error("Invalid argument");
		
		default:
			throw std::runtime_error("Unknown JNI error code");
	}
}
//--------------------------------------------------------------------
jclass jvm::load_class(const std::string& dir_or_jar, const std::string& class_name)
{
	jstring path_string = penv->NewStringUTF(dir_or_jar.c_str());
	jstring class_name_string = penv->NewStringUTF(class_name.c_str());
	auto class_obj = reinterpret_cast<jclass>(penv->CallStaticObjectMethod(ObjectLoader_class, loadObject_method, path_string, class_name_string));
	check_and_throw_jvm_exception(this, penv);
	penv->DeleteLocalRef(path_string);
	penv->DeleteLocalRef(class_name_string);
	
	return class_obj;
}
//--------------------------------------------------------------------
void jvm::free_class(jclass obj)
{
	penv->DeleteLocalRef(obj);
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(jthrowable throwable)
{
	penv->ExceptionClear();
	
	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	check_and_throw_jvm_exception(this, penv);
	//jmethodID mid_throwable_getCause =	penv->GetMethodID(throwable_class,"getCause","()Ljava/lang/Throwable;");
	//jmethodID mid_throwable_getStackTrace =	penv->GetMethodID(throwable_class,"getStackTrace","()[Ljava/lang/StackTraceElement;");
	jmethodID throwable_toString = penv->GetMethodID(throwable_class,"toString","()Ljava/lang/String;");
	check_and_throw_jvm_exception(this, penv);
	
	//jclass frame_class = penv->FindClass("java/lang/StackTraceElement");
	//jmethodID mid_frame_toString = penv->GetMethodID(frame_class,"toString","()Ljava/lang/String;");
	
	jobject str = penv->CallObjectMethod(throwable, throwable_toString);
	scope_guard sg([&](){ penv->DeleteLocalRef(str);	});
	
	check_and_throw_jvm_exception(this, penv);
	std::string res(penv->GetStringUTFChars((jstring)str, nullptr));
	
	return res;
}
//--------------------------------------------------------------------