#include "jvm.h"
#include <stdexcept>
#include <sstream>
#include <utils/scope_guard.hpp>
#include <utils/expand_env.h>
#include <utils/function_path_parser.h>
#include <utils/foreign_function.h>

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
		
		// TODO: Load METAFFI module
		// TODO: currently calling from Java main host to Java code via MetaFFI won't work (nor it is recommended).
		
		
		return;
	}
	
	// create new JVM
	
	std::stringstream ss;
	ss << "-Djava.class.path=" << std::getenv("METAFFI_HOME") << "/xllr.openjdk.bridge.jar" << ":" << std::getenv("METAFFI_HOME") << "/javaparser-core-3.24.4.jar" << ":" << std::getenv("METAFFI_HOME") << "/JavaExtractor_MetaFFIGuest.jar" << ":" << std::getenv("METAFFI_HOME") << "/JavaExtractor.jar";
	printf("JVM classpath: %s\n", ss.str().c_str());
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
	is_destroy = true;
	
}
//--------------------------------------------------------------------
void jvm::load_object_loader(JNIEnv* penv, jclass* object_loader_class, jmethodID* load_object)
{
	// load ObjectLoader
	*object_loader_class = penv->FindClass("metaffi/ObjectLoader");
	check_and_throw_jvm_exception(this, penv, *object_loader_class);
	
	*load_object = penv->GetStaticMethodID(*object_loader_class, "loadObject", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");
	check_and_throw_jvm_exception(this, penv, *load_object);
}
//--------------------------------------------------------------------
void jvm::fini()
{
	if(this->pjvm && is_destroy)
	{
		this->pjvm->DestroyJavaVM();
		this->pjvm = nullptr;
	}
}
//--------------------------------------------------------------------
// returns releaser
std::function<void()> jvm::get_environment(JNIEnv** env)
{
	bool did_attach_thread = false;
	// Check if the current thread is attached to the VM
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
jclass jvm::load_class(const std::string& dir_or_jar, const std::string& class_name)
{
	JNIEnv* penv;
	auto release_env = this->get_environment(&penv);
	scope_guard sg([&](){ release_env(); });
	
	jstring path_string = penv->NewStringUTF(dir_or_jar.c_str());
	if(!path_string)
	{
		throw std::runtime_error("Failed to create new UTF string");
	}
	
	jstring class_name_string = penv->NewStringUTF(class_name.c_str());
	if(!class_name_string)
	{
		throw std::runtime_error("Failed to create new UTF string");
	}
	
	jclass object_loader_class;
	jmethodID load_object;
	this->load_object_loader(penv, &object_loader_class, &load_object);
	
	auto class_obj = reinterpret_cast<jclass>(penv->CallStaticObjectMethod(object_loader_class, load_object, path_string, class_name_string));
	check_and_throw_jvm_exception(this, penv, class_obj);
	if(!class_obj)
	{
		throw std::runtime_error("Failed to call object loader");
	}
	
	penv->DeleteLocalRef(path_string);
	penv->DeleteLocalRef(class_name_string);
	
	return class_obj;
}
//--------------------------------------------------------------------
void jvm::free_class(jclass obj)
{
	JNIEnv* penv;
	auto release_env = this->get_environment(&penv);
	scope_guard sg([&](){ release_env(); });
	
	penv->DeleteLocalRef(obj);
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
jobject jvm::call_function(jmethodID meth, jclass cls, jobject obj, jobjectArray params)
{
	JNIEnv* env;
	auto releaser = get_environment(&env);
	scope_guard sg([&releaser](){ releaser(); });
	
	// TODO: check if there's a JVM exception
	
	if(obj)
	{
		env->CallObjectMethod(obj, meth, params ? params : nullptr);
	}
	else
	{
		env->CallStaticObjectMethod(cls, meth, params ? params : nullptr);
	}
	
	return nullptr;
}
//--------------------------------------------------------------------
jvm::operator JavaVM*()
{
	return this->pjvm;
}
//--------------------------------------------------------------------