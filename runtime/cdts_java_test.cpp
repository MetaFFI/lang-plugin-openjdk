#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include <jni.h>
#include <runtime/runtime_plugin_api.h>
#include <filesystem>
#include <runtime/cdts_wrapper.h>
#include "runtime_id.h"
#include "cdts_java_wrapper.h"
#include "jvm.h"
#include "jobjectArray_wrapper.h"
#include "jbyte_wrapper.h"
#include <csignal>

JNIEnv* env = nullptr;

struct GlobalSetup
{
	jvm* vm = nullptr;
	
	GlobalSetup()
	{
		vm = new jvm();
		auto close_env = vm->get_environment(&env);
	}
	
	~GlobalSetup()
	{
		vm->fini();
		vm = nullptr;
	}
};

static GlobalSetup setup;

TEST_CASE( "Traverse 2D bytes array")
{
	cdt cdt;
	cdt.type = metaffi_uint8_array_type;
	cdt.cdt_val.metaffi_uint8_array_val.dimension = 2;
	cdt.cdt_val.metaffi_uint8_array_val.length = 3;
	cdt.cdt_val.metaffi_uint8_array_val.arr = new cdt_metaffi_uint8_array[3];
	
	cdt.cdt_val.metaffi_uint8_array_val.arr[0].dimension = 1;
	cdt.cdt_val.metaffi_uint8_array_val.arr[0].length = 2;
	cdt.cdt_val.metaffi_uint8_array_val.arr[0].vals = new metaffi_uint8[2];
	cdt.cdt_val.metaffi_uint8_array_val.arr[0].vals[0] = 1;
	cdt.cdt_val.metaffi_uint8_array_val.arr[0].vals[1] = 2;
	
	cdt.cdt_val.metaffi_uint8_array_val.arr[1].dimension = 1;
	cdt.cdt_val.metaffi_uint8_array_val.arr[1].length = 3;
	cdt.cdt_val.metaffi_uint8_array_val.arr[1].vals = new metaffi_uint8[3];
	cdt.cdt_val.metaffi_uint8_array_val.arr[1].vals[0] = 3;
	cdt.cdt_val.metaffi_uint8_array_val.arr[1].vals[1] = 4;
	cdt.cdt_val.metaffi_uint8_array_val.arr[1].vals[2] = 5;
	
	cdt.cdt_val.metaffi_uint8_array_val.arr[2].dimension = 1;
	cdt.cdt_val.metaffi_uint8_array_val.arr[2].length = 4;
	cdt.cdt_val.metaffi_uint8_array_val.arr[2].vals = new metaffi_uint8[4];
	cdt.cdt_val.metaffi_uint8_array_val.arr[2].vals[0] = 6;
	cdt.cdt_val.metaffi_uint8_array_val.arr[2].vals[1] = 7;
	cdt.cdt_val.metaffi_uint8_array_val.arr[2].vals[2] = 8;
	cdt.cdt_val.metaffi_uint8_array_val.arr[2].vals[3] = 9;
	
	
	cdts_java_wrapper cdts_j(&cdt, 1);
	jvalue jval = cdts_j.to_jvalue(env, 0);
	
	jobjectArray_wrapper jarr(env, (jobjectArray)jval.l);
	
	REQUIRE(jarr.size() == 3);
	
	jbyte* arr0 = env->GetByteArrayElements((jbyteArray)jarr.get(0), nullptr);
	jbyte* arr1 = env->GetByteArrayElements((jbyteArray)jarr.get(1), nullptr);
	jbyte* arr2 = env->GetByteArrayElements((jbyteArray)jarr.get(2), nullptr);
	
	REQUIRE(env->GetArrayLength((jbyteArray)jarr.get(0)) == 2);
	REQUIRE(arr0[0] == 1);
	REQUIRE(arr0[1] == 2);
	
	REQUIRE(env->GetArrayLength((jbyteArray)jarr.get(1)) == 3);
	REQUIRE(arr1[0] == 3);
	REQUIRE(arr1[1] == 4);
	REQUIRE(arr1[2] == 5);
	
	REQUIRE(env->GetArrayLength((jbyteArray)jarr.get(2)) == 4);
	REQUIRE(arr2[0] == 6);
	REQUIRE(arr2[1] == 7);
	REQUIRE(arr2[2] == 8);
	REQUIRE(arr2[3] == 9);

}


TEST_CASE("Traverse 1D bytes array")
{
	cdt cdt;
	cdt.type = metaffi_uint8_array_type;
	cdt.cdt_val.metaffi_uint8_array_val.dimension = 1;
	cdt.cdt_val.metaffi_uint8_array_val.vals = new metaffi_uint8[3]{1, 2, 3};
	cdt.cdt_val.metaffi_uint8_array_val.length = 3;

	cdts_java_wrapper cdts_j(&cdt, 1);
	jvalue jval = cdts_j.to_jvalue(env, 0);

	REQUIRE(env->GetArrayLength((jbyteArray)jval.l) == 3);
	jbyte* bytes = env->GetByteArrayElements((jbyteArray)jval.l, nullptr);
	
	REQUIRE((metaffi_uint8)bytes[0] == 1);
	REQUIRE((metaffi_uint8)bytes[1] == 2);
	REQUIRE((metaffi_uint8)bytes[2] == 3);
}

TEST_CASE("Traverse 1D float array")
{
	cdt cdt;
	cdt.type = metaffi_float32_array_type;
	cdt.cdt_val.metaffi_float32_array_val.dimension = 1;
	cdt.cdt_val.metaffi_float32_array_val.vals = new metaffi_float32[3]{1.0f, 2.0f, 3.0f};
	cdt.cdt_val.metaffi_float32_array_val.length = 3;

	cdts_java_wrapper cdts_j(&cdt, 1);
	jvalue jval = cdts_j.to_jvalue(env, 0);
	
	REQUIRE(env->GetArrayLength((jarray)jval.l) == 3);
	jfloat* floats = env->GetFloatArrayElements((jfloatArray)jval.l, nullptr);

	REQUIRE((metaffi_float32)floats[0] == 1.0f);
	REQUIRE((metaffi_float32)floats[1] == 2.0f);
	REQUIRE((metaffi_float32)floats[2] == 3.0f);
}


TEST_CASE("Traverse 3D array")
{
	cdt cdt;
	cdt.type = metaffi_float32_array_type;
	cdt.cdt_val.metaffi_float32_array_val.dimension = 3;
	cdt.cdt_val.metaffi_float32_array_val.arr = new cdt_metaffi_float32_array[2]{};
	cdt.cdt_val.metaffi_float32_array_val.length = 2;

	cdt.cdt_val.metaffi_float32_array_val.arr[0].dimension = 2;
	cdt.cdt_val.metaffi_float32_array_val.arr[0].length = 2;
	cdt.cdt_val.metaffi_float32_array_val.arr[0].arr = new cdt_metaffi_float32_array[2]{};
	cdt.cdt_val.metaffi_float32_array_val.arr[1].dimension = 2;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].length = 3;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr = new cdt_metaffi_float32_array[3]{};

	cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].dimension = 1;
	cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].length = 4;
	cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].vals = new metaffi_float32[4]{1.0f, 2.0f, 3.0f, 4.0f};

	cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[1].dimension = 1;
	cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[1].length = 2;
	cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[1].vals = new metaffi_float32[2]{5.0f, 6.0f};

	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].dimension = 1;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].length = 3;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].vals = new metaffi_float32[3]{7.0f, 8.0f, 9.0f};

	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[1].dimension = 1;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[1].length = 2;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[1].vals = new metaffi_float32[2]{10.0f, 11.0f};

	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[2].dimension = 1;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[2].length = 1;
	cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[2].vals = new metaffi_float32[1]{12.0f};

	cdts_java_wrapper cdts_j(&cdt, 1);
	jvalue jval = cdts_j.to_jvalue(env, 0);
	
	jobjectArray_wrapper jarr(env, (jobjectArray)jval.l);
	REQUIRE(jarr.size() == 2);

	jobjectArray_wrapper jarr2dim1(env, (jobjectArray)jarr.get(0));
	jobjectArray_wrapper jarr2dim2(env, (jobjectArray)jarr.get(1));

	REQUIRE(jarr2dim1.size() == 2);
	REQUIRE(jarr2dim2.size() == 3);

	jfloatArray jarr1dim00 = (jfloatArray)jarr2dim1.get(0);
	jfloatArray jarr1dim01 = (jfloatArray)jarr2dim1.get(1);
	
	jfloatArray jarr1dim10 = (jfloatArray)jarr2dim2.get(0);
	jfloatArray jarr1dim11 = (jfloatArray)jarr2dim2.get(1);
	jfloatArray jarr1dim12 = (jfloatArray)jarr2dim2.get(2);
	
	REQUIRE(env->GetArrayLength(jarr1dim00) == 4);
	REQUIRE(env->GetArrayLength(jarr1dim01) == 2);
	REQUIRE(env->GetArrayLength(jarr1dim10) == 3);
	REQUIRE(env->GetArrayLength(jarr1dim11) == 2);
	REQUIRE(env->GetArrayLength(jarr1dim12) == 1);
	
	jfloat* lst1dim00 = env->GetFloatArrayElements(jarr1dim00, nullptr);
	jfloat* lst1dim01 = env->GetFloatArrayElements(jarr1dim01, nullptr);
	jfloat* lst1dim10 = env->GetFloatArrayElements(jarr1dim10, nullptr);
	jfloat* lst1dim11 = env->GetFloatArrayElements(jarr1dim11, nullptr);
	jfloat* lst1dim12 = env->GetFloatArrayElements(jarr1dim12, nullptr);
	
	REQUIRE((metaffi_float32)lst1dim00[0] == 1.0f);
	REQUIRE((metaffi_float32)lst1dim00[1] == 2.0f);
	REQUIRE((metaffi_float32)lst1dim00[2] == 3.0f);
	REQUIRE((metaffi_float32)lst1dim00[3] == 4.0f);
	REQUIRE((metaffi_float32)lst1dim01[0] == 5.0f);
	REQUIRE((metaffi_float32)lst1dim01[1] == 6.0f);
	REQUIRE((metaffi_float32)lst1dim10[0] == 7.0f);
	REQUIRE((metaffi_float32)lst1dim10[1] == 8.0f);
	REQUIRE((metaffi_float32)lst1dim10[2] == 9.0f);
	REQUIRE((metaffi_float32)lst1dim11[0] == 10.0f);
	REQUIRE((metaffi_float32)lst1dim11[1] == 11.0f);
	REQUIRE((metaffi_float32)lst1dim12[0] == 12.0f);
}

TEST_CASE("Construct 1D bytes array")
{
	uint8_t arr[] = {1, 2, 3};
	jbyteArray jarr = env->NewByteArray(3);
	env->SetByteArrayRegion(jarr, 0, 3, (jbyte*)arr);
	
	cdt cdt;
	cdt.type = metaffi_uint8_array_type;

	cdts_java_wrapper jcdts(&cdt, 1);

	metaffi_type_info info = metaffi_type_info{metaffi_uint8_array_type, nullptr, 0, 1};
	jvalue cdt_val;
	cdt_val.l = jarr;
	jcdts.from_jvalue(env, cdt_val, info, 0);

	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.dimension == 1);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.length == 3);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.vals[0] == 1);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.vals[1] == 2);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.vals[2] == 3);
}

TEST_CASE("Construct 2D bytes array")
{
	uint8_t arr1[] = {1, 2, 3};
	jbyteArray jarr1 = env->NewByteArray(3);
	env->SetByteArrayRegion(jarr1, 0, 3, (jbyte*)arr1);
	
	uint8_t arr2[] = {4, 5, 6};
	jbyteArray jarr2 = env->NewByteArray(3);
	env->SetByteArrayRegion(jarr2, 0, 3, (jbyte*)arr2);

	jobjectArray_wrapper jarr(env, env->NewObjectArray(2, env->FindClass("[B"), nullptr));
	jarr.set(0, jarr1);
	jarr.set(1, jarr2);
	
	cdt cdt;
	cdt.type = metaffi_uint8_array_type;

	cdts_java_wrapper cdts(&cdt, 1);

	metaffi_type_info info = metaffi_type_info{metaffi_uint8_array_type, nullptr, 0, 2};
	jvalue jval;
	jval.l = (jobject)jarr;
	cdts.from_jvalue(env, jval, info, 0);

	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.dimension == 2);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.length == 2);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[0].length == 3);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[0].vals[0] == 1);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[0].vals[1] == 2);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[0].vals[2] == 3);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[1].length == 3);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[1].vals[0] == 4);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[1].vals[1] == 5);
	REQUIRE(cdt.cdt_val.metaffi_uint8_array_val.arr[1].vals[2] == 6);

}

TEST_CASE("Construct 1D array")
{
	jfloatArray jarr = env->NewFloatArray(3);
	jfloat arr[] = {1.0f, 2.0f, 3.0f};
	env->SetFloatArrayRegion(jarr, 0, 3, arr);
	
	cdt cdt;
	cdt.type = metaffi_float32_array_type;

	cdts_java_wrapper cdts(&cdt, 1);

	metaffi_type_info info = metaffi_type_info{metaffi_float32_array_type, nullptr, 0, 1};
	jvalue jval;
	jval.l = (jobject)jarr;
	cdts.from_jvalue(env, jval, info, 0);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.dimension == 1);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.length == 3);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.vals[0] == 1.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.vals[1] == 2.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.vals[2] == 3.0f);
}

TEST_CASE("Construct 3D array")
{
	
	jfloatArray jarr1dim00 = env->NewFloatArray(4);
	jfloat arr1dim00[] = {1.0f, 2.0f, 3.0f, 4.0f};
	env->SetFloatArrayRegion(jarr1dim00, 0, 4, arr1dim00);
	
	jfloatArray jarr1dim01 = env->NewFloatArray(2);
	jfloat arr1dim01[] = {5.0f, 6.0f};
	env->SetFloatArrayRegion(jarr1dim01, 0, 2, arr1dim01);
	
	jfloatArray jarr1dim10 = env->NewFloatArray(3);
	jfloat arr1dim10[] = {7.0f, 8.0f, 9.0f};
	env->SetFloatArrayRegion(jarr1dim10, 0, 3, arr1dim10);
	
	jfloatArray jarr1dim11 = env->NewFloatArray(2);
	jfloat arr1dim11[] = {10.0f, 11.0f};
	env->SetFloatArrayRegion(jarr1dim11, 0, 2, arr1dim11);
	
	jfloatArray jarr1dim12 = env->NewFloatArray(1);
	jfloat arr1dim12[] = {12.0f};
	env->SetFloatArrayRegion(jarr1dim12, 0, 1, arr1dim12);
	
	
	
	jobjectArray_wrapper jarr2dim0(env, env->NewObjectArray(2, env->FindClass("[F"), nullptr));
	jarr2dim0.set(0, jarr1dim00);
	jarr2dim0.set(1, jarr1dim01);
	
	jobjectArray_wrapper jarr2dim1(env, env->NewObjectArray(3, env->FindClass("[F"), nullptr));
	jarr2dim1.set(0, jarr1dim10);
	jarr2dim1.set(1, jarr1dim11);
	jarr2dim1.set(2, jarr1dim12);
	
	jobjectArray_wrapper jarr(env, env->NewObjectArray(2, env->FindClass("[[F"), nullptr));
	jarr.set(0, (jobject)jarr2dim0);
	jarr.set(1, (jobject)jarr2dim1);
	
	cdt cdt;
	cdt.type = metaffi_float32_array_type;

	cdts_java_wrapper jcdts(&cdt, 1);

	metaffi_type_info info = metaffi_type_info{metaffi_float32_array_type, nullptr, 0, 3};
	jvalue cdt_val;
	cdt_val.l = (jobject)jarr;
	jcdts.from_jvalue(env, cdt_val, info, 0);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.dimension == 3);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.length == 2);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].dimension == 2);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].length == 2);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].dimension == 1);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].length == 4);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].vals[0] == 1.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].vals[1] == 2.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].vals[2] == 3.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[0].vals[3] == 4.0f);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[1].dimension == 1);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[1].length == 2);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[1].vals[0] == 5.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[0].arr[1].vals[1] == 6.0f);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].dimension == 2);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].length == 3);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].dimension == 1);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].length == 3);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].vals[0] == 7.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].vals[1] == 8.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[0].vals[2] == 9.0f);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[1].dimension == 1);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[1].length == 2);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[1].vals[0] == 10.0f);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[1].vals[1] == 11.0f);

	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[2].dimension == 1);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[2].length == 1);
	REQUIRE(cdt.cdt_val.metaffi_float32_array_val.arr[1].arr[2].vals[0] == 12.0f);
}