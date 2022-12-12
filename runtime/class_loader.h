#include <jni.h>
#include <string>
#include <vector>

extern "C" jclass load_class(JNIEnv* env, const std::vector<std::string>& path, const char* class_name);
