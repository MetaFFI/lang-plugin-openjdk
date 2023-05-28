#include "jvm.h"
#include <stdexcept>
#include <sstream>
#include <utils/scope_guard.hpp>
#include <utils/expand_env.h>
#include <utils/function_path_parser.h>
#include <utils/foreign_function.h>

#define EXCEPTION_ACCESS_VIOLATION 0xc0000005

using namespace metaffi::utils;

//--------------------------------------------------------------------
jvm::jvm()
{
	// if there's a JVM already loaded, get it.
	jsize nVMs;
	check_throw_error(JNI_GetCreatedJavaVMs(nullptr, 0, &nVMs));
	JNIEnv* penv = nullptr;
	if(nVMs > 0) // JVM already exists
	{
		check_throw_error(JNI_GetCreatedJavaVMs(&this->pjvm, 1, &nVMs));
		
		auto release_env = this->get_environment(&penv);
		scope_guard sg([&](){ release_env(); });
		
		return;
	}
	// create new JVM
	
	//std::stringstream ss;
	//ss << "-Djava.class.path=" << std::getenv("METAFFI_HOME") << "/xllr.openjdk.bridge.jar" << ":" << std::getenv("METAFFI_HOME") << "/javaparser-core-3.24.4.jar" << ":" << std::getenv("METAFFI_HOME") << "/JavaExtractor_MetaFFIGuest.jar" << ":" << std::getenv("METAFFI_HOME") << "/JavaExtractor.jar";
	//printf("JVM classpath: %s\n", ss.str().c_str());
	//std::string options_string = ss.str();
	//JavaVMOption* options = new JavaVMOption[3];
	//options[0].optionString = (char*)options_string.c_str();

	// set initialization args
	JavaVMInitArgs vm_args = {0};
	vm_args.version = JNI_VERSION_10;
	vm_args.nOptions = 0;
	vm_args.options = nullptr;
	vm_args.ignoreUnrecognized = JNI_FALSE;
	// load jvm
	
	// FOR WINDOWS: In order from this code to run from Go executable - "runtime.testingWER" must be set to true!!!!
	jint res = JNI_CreateJavaVM(&this->pjvm, (void**) &penv, &vm_args);
	check_throw_error(res);
	is_destroy = true;
}
//--------------------------------------------------------------------
void jvm::fini()
{
	if(this->pjvm && is_destroy)
	{
		// this->pjvm->DestroyJavaVM(); // TODO: Check why it gets stuck !
		this->pjvm = nullptr;
	}
}
//--------------------------------------------------------------------
// returns releaser
std::function<void()> jvm::get_environment(JNIEnv** env)
{
	bool did_attach_thread = false;
	// Check if th
	// e current thread is attached to the VM
	auto get_env_result = pjvm->GetEnv((void**)env, JNI_VERSION_10);
	if (get_env_result == JNI_EDETACHED)
	{
		if(pjvm->AttachCurrentThread((void**)*env, nullptr) == JNI_OK)
		{
			did_attach_thread = true;
		}
		else
		{
			// Failed to attach thread. Throw an exception if you want to.
			throw std::runtime_error("Failed to attach environment to current thread");
		}
	}
	else if (get_env_result == JNI_EVERSION)
	{
		// Unsupported JNI version. Throw an exception if you want to.
		throw std::runtime_error("Failed to get JVM environment - unsupported JNI version");
	}
	
	return did_attach_thread ? std::function<void()>([this](){ pjvm->DetachCurrentThread(); }) : [](){};
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
void jvm::load_function_path(const std::string& function_path, jclass* cls, jmethodID* meth)
{
	JNIEnv* penv;
	auto release_env = get_environment(&penv);
	scope_guard sg([&](){ release_env(); });
	
	metaffi::utils::function_path_parser fp(function_path);
	
	// get guest module
	//*cls = this->load_class(fp[function_path_entry_metaffi_guest_lib],
    //                               std::string("metaffi_guest/")+fp[function_path_class_entrypoint_function]); // prepend entry point package name;
	*cls = penv->FindClass((std::string("metaffi_guest/")+fp[function_path_entrypoint_class]).c_str());
	check_and_throw_jvm_exception(this, penv, *cls);
	
	*meth = penv->GetStaticMethodID(*cls, (fp[function_path_entry_entrypoint_function]).c_str(), ("(J)V"));
	check_and_throw_jvm_exception(this, penv, *meth);
	
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(jthrowable throwable)
{
	JNIEnv* penv;
	auto release_env = this->get_environment(&penv);
	scope_guard sg_env([&](){ release_env(); });
	
	penv->ExceptionClear();
	
	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	check_and_throw_jvm_exception(this, penv, throwable_class);
	
	jmethodID throwable_toString = penv->GetMethodID(throwable_class,"toString","()Ljava/lang/String;");
	check_and_throw_jvm_exception(this, penv, throwable_toString);
	
	jobject str = penv->CallObjectMethod(throwable, throwable_toString);
	check_and_throw_jvm_exception(this, penv, str);
	
	scope_guard sg([&](){ penv->DeleteLocalRef(str); });
	std::string res(penv->GetStringUTFChars((jstring)str, nullptr));
	
	return res;
}
//--------------------------------------------------------------------
jvm::operator JavaVM*()
{
	return this->pjvm;
}
//--------------------------------------------------------------------