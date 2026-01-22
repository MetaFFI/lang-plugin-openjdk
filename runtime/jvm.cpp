#include "jvm.h"
#include <stdexcept>
#include <sstream>
#include <utils/scope_guard.hpp>
#include <utils/entity_path_parser.h>
#include <utils/foreign_function.h>
#include "exception_macro.h"

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


	// read classpath environment variable and set it, including "." as default.

#ifdef _WIN32
#define SEPARATOR ';'
#else
#define SEPARATOR ':'
#endif

	if(std::getenv("METAFFI_HOME") == nullptr)
	{
		throw std::runtime_error("METAFFI_HOME environment variable is not set");
	}
	
	std::stringstream ss;
	ss << "-Djava.class.path=." << SEPARATOR << ".." << SEPARATOR << std::getenv("METAFFI_HOME") << "/jvm/xllr.jvm.bridge.jar";
	const char* classpath = std::getenv("CLASSPATH");
	if (classpath && strlen(classpath) > 0) {
	    ss << SEPARATOR << classpath;
	}

	printf("JVM classpath: %s\n", ss.str().c_str());
	
	std::string cp_option = ss.str();

	// set initialization args
	JavaVMInitArgs vm_args = {0};
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = 1;
	vm_args.options = new JavaVMOption[1];
	vm_args.options[0].optionString = (char*)cp_option.c_str();
	vm_args.ignoreUnrecognized = JNI_FALSE;

	//printf("JVM classpath: %s\n", vm_args.options[0].optionString);

	// load jvm
	
	// FOR WINDOWS: due to bug in Go, in order to load JVM from Go executable - lastcontinuehandler() in signal_windows.go
	// must return _EXCEPTION_CONTINUE_SEARCH
	// https://github.com/golang/go/issues/58542

	jint res = JNI_CreateJavaVM(&this->pjvm, (void**) &penv, &vm_args);
	delete[] vm_args.options;
	vm_args.options = nullptr;
	check_throw_error(res);
	is_destroy = true;
}
//--------------------------------------------------------------------
void jvm::fini()
{
	if(this->pjvm && is_destroy)
	{
		jint res = this->pjvm->DestroyJavaVM(); // TODO: Check why it gets stuck !
		if(res != JNI_OK){
			printf("Failed to destroy JVM: %ld\n", res);
		}
		
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
	auto get_env_result = pjvm->GetEnv((void**)env, JNI_VERSION_1_4);
	if (get_env_result == JNI_EDETACHED)
	{
		if(pjvm->AttachCurrentThread((void**)env, nullptr) == JNI_OK)
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
void jvm::load_entity_path(const std::string& entity_path, jclass* cls, jmethodID* meth)
{
	JNIEnv* penv;
	auto release_env = get_environment(&penv);
	scope_guard sg([&](){ release_env(); });
	
	metaffi::utils::entity_path_parser fp(entity_path);
	
	// get guest module
	//*cls = this->load_class(fp[entity_path_entry_metaffi_guest_lib],
    //                               std::string("metaffi_guest/")+fp[entity_path_class_entrypoint_function]); // prepend entry point package name;
	*cls = penv->FindClass((std::string("metaffi_guest/")+fp[entity_path_entrypoint_class]).c_str());
	check_and_throw_jvm_exception(penv, *cls);
	
	*meth = penv->GetStaticMethodID(*cls, (fp[entity_path_entry_entrypoint_function]).c_str(), ("(J)V"));
	check_and_throw_jvm_exception(penv, *meth);
	
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(jthrowable throwable)
{
	JNIEnv* penv;
	auto release_env = this->get_environment(&penv);
	scope_guard sg_env([&](){ release_env(); });

	return get_exception_description(penv, throwable);
}
//--------------------------------------------------------------------
std::string jvm::get_exception_description(JNIEnv* penv, jthrowable throwable)
{
	penv->ExceptionClear();

	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	check_and_throw_jvm_exception(penv, throwable_class);

	jmethodID throwable_toString = penv->GetMethodID(throwable_class,"toString","()Ljava/lang/String;");
	check_and_throw_jvm_exception(penv, throwable_toString);

	jobject str = penv->CallObjectMethod(throwable, throwable_toString);
	check_and_throw_jvm_exception(penv, str);

	scope_guard sg([&](){ penv->DeleteLocalRef(str); });
	
	const char* cstr = penv->GetStringUTFChars((jstring)str, nullptr);
	std::string res(cstr);
	penv->ReleaseStringUTFChars((jstring)str, cstr);

	return res;
}
//--------------------------------------------------------------------
jvm::operator JavaVM*()
{
	return this->pjvm;
}
