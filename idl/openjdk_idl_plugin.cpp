#include <iostream>
#include <jni.h>
#include <memory>
#include <string>

// Global JVM pointer
static JavaVM* g_jvm = nullptr;
static JNIEnv* g_env = nullptr;

// Global class references
static jclass g_javaExtractorClass = nullptr;
static jclass g_stringClass = nullptr;

// JNI method IDs
static jmethodID g_javaExtractorConstructor = nullptr;
static jmethodID g_extractMethod = nullptr;
static jmethodID g_toMetaFFIJSONMethod = nullptr;

// Initialize JNI environment
bool initializeJNI()
{
	if(g_jvm != nullptr)
	{
		return true;// Already initialized
	}

	// JVM options - include all necessary JAR files and class directories
	JavaVMOption options[3];
	options[0].optionString = const_cast<char*>("-Djava.class.path=libs/gson-2.10.1.jar;libs/asm-9.6.jar;libs/asm-tree-9.6.jar;libs/javaparser-core-3.24.4.jar;src/java_extractor");
	options[1].optionString = const_cast<char*>("-Dfile.encoding=UTF-8");
	options[2].optionString = const_cast<char*>("-Xmx512m");

	JavaVMInitArgs vm_args;
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = 3;
	vm_args.options = options;
	vm_args.ignoreUnrecognized = JNI_FALSE;

	// Create JVM
	jint result = JNI_CreateJavaVM(&g_jvm, (void**) &g_env, &vm_args);
	if(result != JNI_OK)
	{
		std::cerr << "Failed to create JVM" << std::endl;
		return false;
	}

	// Get class references
	g_javaExtractorClass = g_env->FindClass("java_extractor/JavaExtractor");
	if(g_javaExtractorClass == nullptr)
	{
		std::cerr << "Failed to find JavaExtractor class" << std::endl;
		// Check for JNI exception
		jthrowable exception = g_env->ExceptionOccurred();
		if(exception != nullptr)
		{
			g_env->ExceptionDescribe();
			g_env->ExceptionClear();
		}
		return false;
	}
	g_javaExtractorClass = (jclass) g_env->NewGlobalRef(g_javaExtractorClass);

	g_stringClass = g_env->FindClass("java/lang/String");
	if(g_stringClass == nullptr)
	{
		std::cerr << "Failed to find String class" << std::endl;
		return false;
	}
	g_stringClass = (jclass) g_env->NewGlobalRef(g_stringClass);

	// Get method IDs
	g_javaExtractorConstructor = g_env->GetMethodID(g_javaExtractorClass, "<init>", "(Ljava/lang/String;)V");
	if(g_javaExtractorConstructor == nullptr)
	{
		std::cerr << "Failed to get JavaExtractor constructor" << std::endl;
		jthrowable exception = g_env->ExceptionOccurred();
		if(exception != nullptr)
		{
			g_env->ExceptionDescribe();
			g_env->ExceptionClear();
		}
		return false;
	}

	g_extractMethod = g_env->GetMethodID(g_javaExtractorClass, "extract", "()Ljava_extractor/JavaInfo;");
	if(g_extractMethod == nullptr)
	{
		std::cerr << "Failed to get extract method" << std::endl;
		jthrowable exception = g_env->ExceptionOccurred();
		if(exception != nullptr)
		{
			g_env->ExceptionDescribe();
			g_env->ExceptionClear();
		}
		return false;
	}

	// Get JavaInfo class and toMetaFFIJSON method
	jclass javaInfoClass = g_env->FindClass("java_extractor/JavaInfo");
	if(javaInfoClass == nullptr)
	{
		std::cerr << "Failed to find JavaInfo class" << std::endl;
		jthrowable exception = g_env->ExceptionOccurred();
		if(exception != nullptr)
		{
			g_env->ExceptionDescribe();
			g_env->ExceptionClear();
		}
		return false;
	}

	g_toMetaFFIJSONMethod = g_env->GetMethodID(javaInfoClass, "toMetaFFIJSON", "()Ljava/lang/String;");
	if(g_toMetaFFIJSONMethod == nullptr)
	{
		std::cerr << "Failed to get toMetaFFIJSON method" << std::endl;
		jthrowable exception = g_env->ExceptionOccurred();
		if(exception != nullptr)
		{
			g_env->ExceptionDescribe();
			g_env->ExceptionClear();
		}
		return false;
	}

	return true;
}

// Cleanup JNI environment
void cleanupJNI()
{
	if(g_env != nullptr)
	{
		if(g_javaExtractorClass != nullptr)
		{
			g_env->DeleteGlobalRef(g_javaExtractorClass);
			g_javaExtractorClass = nullptr;
		}
		if(g_stringClass != nullptr)
		{
			g_env->DeleteGlobalRef(g_stringClass);
			g_stringClass = nullptr;
		}
	}

	if(g_jvm != nullptr)
	{
		g_jvm->DestroyJavaVM();
		g_jvm = nullptr;
		g_env = nullptr;
	}
}

// Convert C string to Java string
jstring cStringToJavaString(const char* cStr)
{
	if(cStr == nullptr)
	{
		return nullptr;
	}
	return g_env->NewStringUTF(cStr);
}

// Convert Java string to C string
char* javaStringToCString(jstring javaStr)
{
	if(javaStr == nullptr)
	{
		return nullptr;
	}

	const char* cStr = g_env->GetStringUTFChars(javaStr, nullptr);
	if(cStr == nullptr)
	{
		return nullptr;
	}

	size_t len = strlen(cStr);
	char* result = new char[len + 1];
	strcpy(result, cStr);

	g_env->ReleaseStringUTFChars(javaStr, cStr);
	return result;
}

// Main plugin entry point
extern "C" {
int parse_idl(const char* source_code, const char* file_path,
              char** out_idl_json, char** out_err)
{

	// Initialize output parameters
	*out_idl_json = nullptr;
	*out_err = nullptr;

	try
	{
		// Initialize JNI if not already done
		if(!initializeJNI())
		{
			*out_err = strdup("Failed to initialize JNI environment");
			return -1;
		}

		// Create Java string for file path
		jstring javaFilePath = cStringToJavaString(file_path);
		if(javaFilePath == nullptr)
		{
			*out_err = strdup("Invalid file path");
			return -1;
		}

		// Create JavaExtractor instance
		jobject javaExtractor = g_env->NewObject(g_javaExtractorClass, g_javaExtractorConstructor, javaFilePath);
		if(javaExtractor == nullptr)
		{
			g_env->DeleteLocalRef(javaFilePath);
			// Check for Java exception
			jthrowable exception = g_env->ExceptionOccurred();
			if(exception != nullptr)
			{
				g_env->ExceptionDescribe();
				g_env->ExceptionClear();
				*out_err = strdup("Java exception during JavaExtractor creation");
			}
			else
			{
				*out_err = strdup("Failed to create JavaExtractor instance");
			}
			return -1;
		}

		// Call extract method
		jobject javaInfo = g_env->CallObjectMethod(javaExtractor, g_extractMethod);
		if(javaInfo == nullptr)
		{
			g_env->DeleteLocalRef(javaExtractor);
			g_env->DeleteLocalRef(javaFilePath);
			// Check for Java exception
			jthrowable exception = g_env->ExceptionOccurred();
			if(exception != nullptr)
			{
				g_env->ExceptionDescribe();
				g_env->ExceptionClear();
				*out_err = strdup("Java exception during extraction");
			}
			else
			{
				*out_err = strdup("Failed to extract Java info");
			}
			return -1;
		}

		// Call toMetaFFIJSON method
		jstring jsonString = (jstring) g_env->CallObjectMethod(javaInfo, g_toMetaFFIJSONMethod);
		if(jsonString == nullptr)
		{
			g_env->DeleteLocalRef(javaInfo);
			g_env->DeleteLocalRef(javaExtractor);
			g_env->DeleteLocalRef(javaFilePath);
			*out_err = strdup("Failed to generate JSON");
			return -1;
		}

		// Convert JSON string to C string
		*out_idl_json = javaStringToCString(jsonString);

		// Cleanup local references
		g_env->DeleteLocalRef(jsonString);
		g_env->DeleteLocalRef(javaInfo);
		g_env->DeleteLocalRef(javaExtractor);
		g_env->DeleteLocalRef(javaFilePath);

		return 0;// Success

	} catch(const std::exception& e)
	{
		*out_err = strdup(e.what());
		return -1;
	} catch(...)
	{
		*out_err = strdup("Unknown error occurred");
		return -1;
	}
}

// Plugin initialization function
void init_plugin()
{
	// JNI will be initialized on first parse_idl call
	std::cout << "OpenJDK IDL Plugin initialized" << std::endl;
}

// Plugin cleanup function
void cleanup_plugin()
{
	cleanupJNI();
	std::cout << "OpenJDK IDL Plugin cleaned up" << std::endl;
}
}