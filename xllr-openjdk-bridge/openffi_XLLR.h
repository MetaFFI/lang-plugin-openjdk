/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class openffi_XLLR */

#ifndef _Included_openffi_XLLR
#define _Included_openffi_XLLR
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     openffi_XLLR
 * Method:    init
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_openffi_XLLR_init
  (JNIEnv *, jobject);

/*
 * Class:     openffi_XLLR
 * Method:    load_runtime_plugin
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_openffi_XLLR_load_1runtime_1plugin
  (JNIEnv *, jobject, jstring);

/*
 * Class:     openffi_XLLR
 * Method:    free_runtime_plugin
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_openffi_XLLR_free_1runtime_1plugin
  (JNIEnv *, jobject, jstring);

/*
 * Class:     openffi_XLLR
 * Method:    load_function
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_openffi_XLLR_load_1function
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     openffi_XLLR
 * Method:    free_function
 * Signature: (Ljava/lang/String;J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_openffi_XLLR_free_1function
  (JNIEnv *, jobject, jstring, jlong);

/*
 * Class:     openffi_XLLR
 * Method:    call
 * Signature: (Ljava/lang/String;J[BLopenffi/CallResult;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_openffi_XLLR_call
  (JNIEnv *, jobject, jstring, jlong, jbyteArray, jobject);

#ifdef __cplusplus
}
#endif
#endif
