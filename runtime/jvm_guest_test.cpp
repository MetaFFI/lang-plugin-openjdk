#include "jvm.h"
#include <iostream>


jclass load_class(jvm* jvm, JNIEnv* env, const std::string& path, const std::string& class_name)
{
	const std::string urlPath = "file://" + path;
	
	// get class loader
	jclass class_loader_class = env->FindClass("java/lang/ClassLoader");
	check_and_throw_jvm_exception(jvm, env, class_loader_class);
	
	jmethodID get_system_class_loader_method = env->GetStaticMethodID(class_loader_class, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
	check_and_throw_jvm_exception(jvm, env, get_system_class_loader_method);
	
	jclass url_class_loader = env->FindClass("java/net/URLClassLoader");
	check_and_throw_jvm_exception(jvm, env, url_class_loader);
	
	jmethodID url_class_loader_constructor = env->GetMethodID(url_class_loader, "<init>", "([Ljava/net/URL;Ljava/lang/ClassLoader;)V");
	check_and_throw_jvm_exception(jvm, env, url_class_loader_constructor);
	
	// classLoaderInstance = ClassLoader.getSystemClassLoader()
	jobject classLoaderInstance = env->CallStaticObjectMethod(class_loader_class, get_system_class_loader_method);
	check_and_throw_jvm_exception(jvm, env, classLoaderInstance);
	
	// new URL[]{ urlInstance }
	jclass url_class = env->FindClass("java/net/URL");
	check_and_throw_jvm_exception(jvm, env, url_class);
	
	jmethodID url_class_constructor = env->GetMethodID(url_class, "<init>", "(Ljava/lang/String;)V");
	check_and_throw_jvm_exception(jvm, env, url_class_constructor);
	
	jobject urlInstance = env->NewObject(url_class, url_class_constructor, env->NewStringUTF(urlPath.c_str()));
	check_and_throw_jvm_exception(jvm, env, urlInstance);
	// TODO: release urlInstance
	
	jobjectArray jarURLArray = env->NewObjectArray(1, url_class, nullptr);
	env->SetObjectArrayElement(jarURLArray, 0, urlInstance);
	// TODO: release jarURLArray
	
	// URLClassLoader childURLClassLoader = new URLClassLoader( jarURLArray, classLoaderInstance ) ;
	jobject childURLClassLoader = env->NewObject(url_class_loader, url_class_loader_constructor, jarURLArray, classLoaderInstance);
	// TODO: release childURLClassLoader
	
	// Class targetClass = Class.forName(class_name, true, child);
	jclass class_class = env->FindClass("java/lang/Class");
	check_and_throw_jvm_exception(jvm, env, class_class);
	
	jmethodID for_name_method = env->GetStaticMethodID(class_class, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
	check_and_throw_jvm_exception(jvm, env, for_name_method);
	
	jobject targetClass = env->CallStaticObjectMethod(class_class, for_name_method, env->NewStringUTF(class_name.c_str()), JNI_TRUE, childURLClassLoader);
	check_and_throw_jvm_exception(jvm, env, targetClass);
	
	// TODO: free in block guard
	env->DeleteLocalRef(childURLClassLoader);
	env->DeleteLocalRef(urlInstance);
	env->DeleteLocalRef(jarURLArray);
	env->DeleteLocalRef(classLoaderInstance);
	
	return (jclass)targetClass;
}

int main()
{
	jvm j;

	JNIEnv* env;
	auto releaser = j.get_environment(&env);
	
	// make sure we can load MetaFFIHandle from xllr.openjdk.bridge.jar
	jclass handle_class = load_class(&j, env, std::string(std::getenv("METAFFI_HOME"))+"/xllr.openjdk.bridge.jar", "metaffi.MetaFFIHandle");
	
	releaser();
}
