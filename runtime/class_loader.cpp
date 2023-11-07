#include "class_loader.h"
#include <set>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>

jclass class_loader_class = nullptr;
jmethodID get_system_class_loader_method = nullptr;
jclass url_class_loader = nullptr;
jmethodID url_class_loader_constructor = nullptr;
jobject classLoaderInstance = nullptr;
jclass url_class = nullptr;
jmethodID url_class_constructor = nullptr;
jclass class_class = nullptr;
jmethodID for_name_method = nullptr;
jmethodID add_url = nullptr;
jobject childURLClassLoader = nullptr;
bool is_bridge_added = false;
std::set<std::string> loaded_paths;
std::unordered_map<std::string,jclass> loaded_classes;

#ifdef _WIN32
std::string file_protocol("file:///");
#else
std::string file_protocol("file://");
#endif


#define check_and_throw_jvm_exception(env, var, before_throw_code) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
env->ExceptionClear();\
before_throw_code \
throw std::runtime_error(err_msg);\
}\
else if(!var)\
{\
before_throw_code; \
throw std::runtime_error("Failed to get " #var);\
}

#define if_exception_throw_jvm_exception(env, before_throw_code) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = get_exception_description(env, env->ExceptionOccurred());\
env->ExceptionClear();\
before_throw_code; \
throw std::runtime_error(err_msg);\
}

struct block_guard
{
	explicit block_guard(std::function<void()> f):f(std::move(f)){}
	~block_guard()
	{
		try{ f(); } catch(...){printf("block_guard function threw an exception\n");}
	}

private:
	std::function<void()> f;
};

std::string get_exception_description(JNIEnv* penv, jthrowable throwable)
{
	jclass throwable_class = penv->FindClass("java/lang/Throwable");
	if(!throwable_class)
	{
		throw std::runtime_error("failed to FindClass java/lang/Throwable");
	}
	
	jclass StringWriter_class = penv->FindClass("java/io/StringWriter");
	if(!StringWriter_class)
	{
		throw std::runtime_error("failed to FindClass java/io/StringWriter");
	}
	
	jclass PrintWriter_class = penv->FindClass("java/io/PrintWriter");
	if(!PrintWriter_class)
	{
		throw std::runtime_error("failed to FindClass java/io/PrintWriter");
	}
	
	jmethodID throwable_printStackTrace = penv->GetMethodID(throwable_class,"printStackTrace","(Ljava/io/PrintWriter;)V");
	if(!throwable_printStackTrace)
	{
		throw std::runtime_error("failed to GetMethodID throwable_printStackTrace");
	}
	
	jmethodID StringWriter_Constructor = penv->GetMethodID(StringWriter_class,"<init>","()V");
	if(!StringWriter_Constructor)
	{
		throw std::runtime_error("failed to GetMethodID StringWriter_Constructor");
	}
	
	jmethodID PrintWriter_Constructor = penv->GetMethodID(PrintWriter_class,"<init>","(Ljava/io/Writer;)V");
	if(!PrintWriter_Constructor)
	{
		throw std::runtime_error("failed to GetMethodID PrintWriter_Constructor");
	}
	
	jmethodID StringWriter_toString = penv->GetMethodID(StringWriter_class,"toString","()Ljava/lang/String;");
	if(!StringWriter_toString)
	{
		throw std::runtime_error("failed to GetMethodID StringWriter_toString");
	}
	
	// StringWriter sw = new StringWriter();
	jobject sw = penv->NewObject(StringWriter_class, StringWriter_Constructor);
	if(!sw)
	{
		throw std::runtime_error("Failed to create StringWriter object");
	}
	
	// PrintWriter pw = new PrintWriter(sw)
	jobject pw = penv->NewObject(PrintWriter_class, PrintWriter_Constructor, sw);
	if(!pw)
	{
		throw std::runtime_error("Failed to create PrintWriter object");
	}
	
	// throwable.printStackTrace(pw);
	penv->CallObjectMethod(throwable, throwable_printStackTrace, pw);
	if(!pw)
	{
		throw std::runtime_error("Failed to call printStackTrace");
	}
	
	// sw.toString()
	jobject str = penv->CallObjectMethod(sw, StringWriter_toString);
	if(!pw)
	{
		throw std::runtime_error("Failed to call printStackTrace");
	}
	
	std::string res(penv->GetStringUTFChars((jstring)str, nullptr));
	
	penv->DeleteLocalRef(sw);
	penv->DeleteLocalRef(pw);
	penv->DeleteLocalRef(str);
	
	return res;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4297)
#endif
jclass load_class(JNIEnv* env, const char* class_path, const char* class_name)
{
	// if class already loaded - return jclass
	if(auto it = loaded_classes.find(class_name); it != loaded_classes.end())
	{
		//printf("+++ already loaded %s\n", class_name);
		return it->second;
	}
	
	// get class loader
	if(!class_loader_class)
	{
		class_loader_class = env->FindClass("java/lang/ClassLoader");
		check_and_throw_jvm_exception(env, class_loader_class,);
	}
	
	if(!get_system_class_loader_method)
	{
		get_system_class_loader_method = env->GetStaticMethodID(class_loader_class, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
		check_and_throw_jvm_exception(env, get_system_class_loader_method,);
	}
	
	if(!url_class_loader)
	{
		url_class_loader = env->FindClass("java/net/URLClassLoader");
		check_and_throw_jvm_exception(env, url_class_loader,);
	}
	
	if(!url_class_loader_constructor)
	{
		url_class_loader_constructor = env->GetMethodID(url_class_loader, "<init>", "([Ljava/net/URL;Ljava/lang/ClassLoader;)V");
		check_and_throw_jvm_exception(env, url_class_loader_constructor,);
	}
	
	if(!add_url)
	{
		add_url = env->GetMethodID(url_class_loader, "addURL", "(Ljava/net/URL;)V");
		check_and_throw_jvm_exception(env, add_url,);
	}
	
	if(!classLoaderInstance)
	{
		// classLoaderInstance = ClassLoader.getSystemClassLoader()
		classLoaderInstance = env->CallStaticObjectMethod(class_loader_class, get_system_class_loader_method);
		check_and_throw_jvm_exception(env, classLoaderInstance,);
	}
	
	if(!url_class)
	{
		// new URL[]{ urlInstance }
		url_class = env->FindClass("java/net/URL");
		check_and_throw_jvm_exception(env, url_class,);
	}
	
	if(!url_class_constructor)
	{
		url_class_constructor = env->GetMethodID(url_class, "<init>", "(Ljava/lang/String;)V");
		check_and_throw_jvm_exception(env, url_class_constructor,);
	}
	
	if(!class_class)
	{
		// Class targetClass = Class.forName(class_name, true, child);
		class_class = env->FindClass("java/lang/Class");
		check_and_throw_jvm_exception(env, class_class,);
	}
	
	if(!for_name_method)
	{
		for_name_method = env->GetStaticMethodID(class_class, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
		check_and_throw_jvm_exception(env, for_name_method,);
	}
	
	if(!childURLClassLoader)
	{
		// URLClassLoader childURLClassLoader = new URLClassLoader( jarURLArray, classLoaderInstance ) ;
		
		// initialize with "$METAFFI_HOME/xllr.openjdk.bridge.jar"
		std::string openjdk_bridge_url = (std::string("file://") + std::getenv("METAFFI_HOME")) + "/xllr.openjdk.bridge.jar";
		
		jobjectArray jarURLArray = env->NewObjectArray(1, url_class, nullptr); // URL[]{}
		check_and_throw_jvm_exception(env, jarURLArray,);
		env->SetObjectArrayElement(jarURLArray, 0, env->NewObject(url_class, url_class_constructor, env->NewStringUTF(openjdk_bridge_url.c_str())));
		check_and_throw_jvm_exception(env, true,);
		
		childURLClassLoader = env->NewObject(url_class_loader, url_class_loader_constructor, jarURLArray, classLoaderInstance);
		check_and_throw_jvm_exception(env, childURLClassLoader,);
	}
	
	if(!is_bridge_added)
	{
		std::string openjdk_bridge = file_protocol + std::getenv("METAFFI_HOME");
		openjdk_bridge += "/";
		openjdk_bridge += "xllr.openjdk.bridge.jar";
		
#ifdef _WIN32
		boost::replace_all(openjdk_bridge, "\\", "/");
#endif
		
		jobject urlInstance = env->NewObject(url_class, url_class_constructor, env->NewStringUTF(openjdk_bridge.c_str()));
		check_and_throw_jvm_exception(env, urlInstance,);
		env->CallObjectMethod(childURLClassLoader, add_url, urlInstance);
		check_and_throw_jvm_exception(env, true,);
		
		is_bridge_added = true;
	}
	
	std::string tmp;
	std::stringstream ss(class_path);
	std::vector<std::string> classpath_vec;
	while(std::getline(ss, tmp, ';')){
		classpath_vec.push_back(tmp);
	}
	
	// every URL that is NOT loaded - add URL
	for(const auto & i : classpath_vec)
	{
		std::string url_path = file_protocol+i;
		if(loaded_paths.find(url_path) != loaded_paths.end())
		{
			continue;
		}

#ifdef _WIN32
		boost::replace_all(url_path, "\\", "/");
#endif
	
		jobject urlInstance = env->NewObject(url_class, url_class_constructor, env->NewStringUTF(url_path.c_str()));
		check_and_throw_jvm_exception(env, urlInstance,);
		env->CallObjectMethod(childURLClassLoader, add_url, urlInstance);
		check_and_throw_jvm_exception(env, true,);
		
		loaded_paths.insert(url_path);
	}

	jobject targetClass = env->CallStaticObjectMethod(class_class, for_name_method, env->NewStringUTF(class_name), JNI_TRUE, childURLClassLoader);

	check_and_throw_jvm_exception(env, targetClass,);
	loaded_classes[class_name] = (jclass)targetClass;
	
	return (jclass)targetClass;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif