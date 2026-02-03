#include <compiler/idl_plugin_interface.h>
#include <utils/logger.hpp>
#include <jni.h>
#include <memory>
#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>

static auto LOG = metaffi::get_logger("jvm.idl");

class JVMIDLPlugin : public idl_plugin_interface
{
private:
	// JVM pointer
	JavaVM* g_jvm = nullptr;
	JNIEnv* g_env = nullptr;

	// Global class references
	jclass g_javaExtractorClass = nullptr;
	jclass g_stringClass = nullptr;

	// JNI method IDs
	jmethodID g_extractMethod = nullptr;

	bool initializeJNI()
	{
		METAFFI_DEBUG(LOG, "initializeJNI: Starting initialization");
		
		if(g_jvm != nullptr)
		{
			METAFFI_DEBUG(LOG, "initializeJNI: JVM already initialized");
			return true;// Already initialized
		}

		// Get METAFFI_HOME environment variable
		const char* metaffi_home = std::getenv("METAFFI_HOME");
		if(metaffi_home == nullptr)
		{
			METAFFI_ERROR(LOG, "METAFFI_HOME environment variable not set");
			return false;
		}
		
		METAFFI_DEBUG(LOG, "initializeJNI: METAFFI_HOME = {}", metaffi_home);

		// Build classpath with JAR file
		// The JAR includes all dependencies (ASM, Gson) via INCLUDE_JARS
		std::string classpath = std::string(metaffi_home) + "/sdk/idl_compiler/jvm/jvm_idl_compiler.jar";

		METAFFI_DEBUG(LOG, "initializeJNI: Classpath = {}", classpath);

		// JVM options - include all necessary JAR files and class directories
		JavaVMOption options[3];
		options[0].optionString = const_cast<char*>(("-Djava.class.path=" + classpath).c_str());
		options[1].optionString = const_cast<char*>("-Dfile.encoding=UTF-8");
		options[2].optionString = const_cast<char*>("-Xmx512m");

		JavaVMInitArgs vm_args;
		vm_args.version = JNI_VERSION_1_8;
		vm_args.nOptions = 3;
		vm_args.options = options;
		vm_args.ignoreUnrecognized = JNI_FALSE;

		// Create JVM
		METAFFI_DEBUG(LOG, "initializeJNI: Creating JVM...");
		jint result = JNI_CreateJavaVM(&g_jvm, (void**) &g_env, &vm_args);
		if(result != JNI_OK)
		{
			METAFFI_ERROR(LOG, "Failed to create JVM with result: {}", result);
			return false;
		}
		METAFFI_DEBUG(LOG, "initializeJNI: JVM created successfully");

		// Get class references - use new JvmExtractor class
		g_javaExtractorClass = g_env->FindClass("com/metaffi/idl/JvmExtractor");
		if(g_javaExtractorClass == nullptr)
		{
			METAFFI_ERROR(LOG, "Failed to find JvmExtractor class");
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
			METAFFI_ERROR(LOG, "Failed to find String class");
			return false;
		}
		g_stringClass = (jclass) g_env->NewGlobalRef(g_stringClass);

		// Get static extract method
		g_extractMethod = g_env->GetStaticMethodID(g_javaExtractorClass, "extract", 
			"([Ljava/lang/String;)Ljava/lang/String;");
		if(g_extractMethod == nullptr)
		{
			METAFFI_ERROR(LOG, "Failed to get extract static method");
			jthrowable exception = g_env->ExceptionOccurred();
			if(exception != nullptr)
			{
				g_env->ExceptionDescribe();
				g_env->ExceptionClear();
			}
			return false;
		}

		METAFFI_DEBUG(LOG, "initializeJNI: Initialization completed successfully");
		return true;
	}

	void cleanupJNI()
	{
		if(g_jvm != nullptr)
		{
			g_jvm->DestroyJavaVM();
			g_jvm = nullptr;
			g_env = nullptr;
		}
	}

	jstring cStringToJavaString(const char* cStr)
	{
		return g_env->NewStringUTF(cStr);
	}

	char* javaStringToCString(jstring javaStr)
	{
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

public:
	void init() override
	{
		METAFFI_INFO(LOG, "initialized");
	}

	void parse_idl(const char* source_code, uint32_t source_length,
				   const char* file_or_path, uint32_t file_or_path_length,
				   char** out_idl_def_json, uint32_t* out_idl_def_json_length,
				   char** out_err, uint32_t* out_err_len) override
	{
		// Initialize output parameters
		*out_idl_def_json = nullptr;
		*out_idl_def_json_length = 0;
		*out_err = nullptr;
		*out_err_len = 0;

		try
		{
			// Validate input parameters
			if(file_or_path == nullptr)
			{
				std::string error = "file_or_path is null";
				*out_err = (char*) malloc(error.length() + 1);
				if(*out_err != nullptr)
				{
					memcpy(*out_err, error.c_str(), error.length());
					(*out_err)[error.length()] = '\0';
					*out_err_len = error.length();
				}
				return;
			}
			
			if(out_idl_def_json == nullptr || out_err == nullptr)
			{
				return; // Invalid output parameters
			}
			
			// Initialize JVM if not already done
			if(!initializeJNI())
			{
				std::string error = "Failed to initialize JVM";
				*out_err = (char*) malloc(error.length() + 1);
				if(*out_err != nullptr)
				{
					memcpy(*out_err, error.c_str(), error.length());
					(*out_err)[error.length()] = '\0';
					*out_err_len = error.length();
				}
				return;
			}

			// Parse idl_path (may contain multiple paths separated by ;)
			std::string paths_str(file_or_path, file_or_path_length);
			std::vector<std::string> paths;
			
			// Handle both ; and : separators (Windows uses ;, Unix uses :)
			char separator = ';';
			if(paths_str.find(':') != std::string::npos && paths_str.find(';') == std::string::npos)
			{
				separator = ':';
			}
			
			size_t pos = 0;
			while((pos = paths_str.find(separator)) != std::string::npos)
			{
				paths.push_back(paths_str.substr(0, pos));
				paths_str.erase(0, pos + 1);
			}
			if(!paths_str.empty())
			{
				paths.push_back(paths_str);
			}

			// Create Java String array
			jobjectArray pathsArray = g_env->NewObjectArray(paths.size(), g_stringClass, nullptr);
			if(pathsArray == nullptr)
			{
				std::string error = "Failed to create String array";
				*out_err = (char*) malloc(error.length() + 1);
				if(*out_err != nullptr)
				{
					memcpy(*out_err, error.c_str(), error.length());
					(*out_err)[error.length()] = '\0';
					*out_err_len = error.length();
				}
				return;
			}

			for(size_t i = 0; i < paths.size(); i++)
			{
				jstring path = g_env->NewStringUTF(paths[i].c_str());
				g_env->SetObjectArrayElement(pathsArray, i, path);
				g_env->DeleteLocalRef(path);
			}

			// Call static extract method
			jstring result = (jstring) g_env->CallStaticObjectMethod(
				g_javaExtractorClass, g_extractMethod, pathsArray);

			// Check for exceptions
			if(g_env->ExceptionCheck())
			{
				g_env->ExceptionDescribe();
				g_env->ExceptionClear();
				std::string error = "Exception in JvmExtractor.extract()";
				*out_err = (char*) malloc(error.length() + 1);
				if(*out_err != nullptr)
				{
					memcpy(*out_err, error.c_str(), error.length());
					(*out_err)[error.length()] = '\0';
					*out_err_len = error.length();
				}
				g_env->DeleteLocalRef(pathsArray);
				return;
			}

			// Convert result to C string
			if(result != nullptr)
			{
				const char* json_str = g_env->GetStringUTFChars(result, nullptr);
				size_t json_len = strlen(json_str);

				*out_idl_def_json = (char*) malloc(json_len + 1);
				if(*out_idl_def_json != nullptr)
				{
					memcpy(*out_idl_def_json, json_str, json_len);
					(*out_idl_def_json)[json_len] = '\0';
					*out_idl_def_json_length = json_len;
				}

				g_env->ReleaseStringUTFChars(result, json_str);
				g_env->DeleteLocalRef(result);
			}
			else
			{
				std::string error = "JvmExtractor.extract() returned null";
				*out_err = (char*) malloc(error.length() + 1);
				if(*out_err != nullptr)
				{
					memcpy(*out_err, error.c_str(), error.length());
					(*out_err)[error.length()] = '\0';
					*out_err_len = error.length();
				}
			}

			g_env->DeleteLocalRef(pathsArray);

		} catch(const std::exception& e)
		{
			std::string error = e.what();
			*out_err = (char*) malloc(error.length() + 1);
			if(*out_err != nullptr)
			{
				memcpy(*out_err, error.c_str(), error.length());
				(*out_err)[error.length()] = '\0';
				*out_err_len = error.length();
			}
		} catch(...)
		{
			std::string error = "Unknown error occurred";
			*out_err = (char*) malloc(error.length() + 1);
			if(*out_err != nullptr)
			{
				memcpy(*out_err, error.c_str(), error.length());
				(*out_err)[error.length()] = '\0';
				*out_err_len = error.length();
			}
		}
	}

	~JVMIDLPlugin()
	{
		cleanupJNI();
		METAFFI_INFO(LOG, "cleaned up");
	}
};

// Export the plugin instance
extern "C" __declspec(dllexport) idl_plugin_interface* create_plugin()
{
	return new JVMIDLPlugin();
}

// Export init_plugin function for compatibility with the wrapper
extern "C" __declspec(dllexport) void init_plugin()
{
	// This function is called by the wrapper to initialize the plugin
	// The actual initialization is done in the create_plugin function
} 
