/* DO NOT EDIT THIS FILE - it is machine generated */
#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif
/* Header for class metaffi_MetaFFIBridge */

#ifndef _Included_metaffi_MetaFFIBridge
#define _Included_metaffi_MetaFFIBridge
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    load_runtime_plugin
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_load_1runtime_1plugin
		(JNIEnv *, jobject, jstring);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    free_runtime_plugin
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_free_1runtime_1plugin
		(JNIEnv *, jobject, jstring);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    load_function
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;
 */
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_load_1function
		(JNIEnv *, jobject, jstring, jstring, jstring, jobjectArray, jobjectArray);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    free_function
 * Signature: (Ljava/lang/String;J)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_free_1function
		(JNIEnv *, jobject, jstring, jlong);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    xcall_params_ret
 * Signature: (JLjava/lang/Long;)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1params_1ret
		(JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    xcall_no_params_ret
 * Signature: (JLjava/lang/Long;)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1no_1params_1ret
		(JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    xcall_params_no_ret
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1params_1no_1ret
		(JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    xcall_no_params_no_ret
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_xcall_1no_1params_1no_1ret
		(JNIEnv *, jobject, jlong);


/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    alloc_cdts
 * Signature: (BB)J
 */
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_alloc_1cdts
		(JNIEnv *, jobject, jbyte, jbyte);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    get_pcdt
 * Signature: (JB)J
 */
JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_get_1pcdt
		(JNIEnv *, jobject, jlong, jbyte);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    get_object
 * Signature: (J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_metaffi_MetaFFIBridge_get_1object
		(JNIEnv *, jobject, jlong);

/*
 * Class:     metaffi_MetaFFIBridge
 * Method:    remove_object
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_metaffi_MetaFFIBridge_remove_1object
		(JNIEnv *, jobject, jlong);

 /*
  * Class:     metaffi_MetaFFIBridge
  * Method:    java_to_cdts
  * Signature: (JB[Ljava/lang/Object;[J)J
  */
 JNIEXPORT jlong JNICALL Java_metaffi_MetaFFIBridge_java_1to_1cdts
   (JNIEnv *, jobject, jlong, jobjectArray, jlongArray);

 /*
  * Class:     metaffi_MetaFFIBridge
  * Method:    cdts_to_java
  * Signature: (JJ)[Ljava/lang/Object;
  */
 JNIEXPORT jobjectArray JNICALL Java_metaffi_MetaFFIBridge_cdts_1to_1java
   (JNIEnv *, jobject, jlong, jlong);


#ifdef __cplusplus
}
#endif
#endif
