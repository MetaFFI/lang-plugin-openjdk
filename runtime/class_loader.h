#ifdef _MSC_VER
#include <corecrt.h>
#endif
#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif
#include <string>
#include <vector>

// NOTICE: although it is "extern C" the function does throw an exception!
extern "C" jclass load_class(JNIEnv* env, const std::vector<std::string>& path, const char* class_name);
