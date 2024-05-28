#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define NOMINMAX
#include "cdts_java_wrapper.h"
#include "jarray_wrapper.h"
#include "jbyte_wrapper.h"
#include "jvm.h"
#include <doctest/doctest.h>
#include <filesystem>
#include <jni.h>


JNIEnv* env = nullptr;

struct GlobalSetup {
	jvm* vm = nullptr;

	GlobalSetup()
	{
		vm = new jvm();
		auto close_env = vm->get_environment(&env);
	}

	~GlobalSetup()
	{
		vm->fini();
		delete vm;
		vm = nullptr;
	}
};

static GlobalSetup setup;

TEST_SUITE("CDTS Java Tests")
{
	TEST_CASE("Traverse 2D bytes array")
	{
		cdts pcdts(1, 1);                        // array of length 1 and 1 fixed dimension
		pcdts[0].set_new_array(3, 2, metaffi_uint8_type);// array of length 3 and 2 fixed dimensions

		pcdts[0].cdt_val.array_val->arr[0].set_new_array(2, 1, metaffi_uint8_type);// array of length 2 and 1 fixed dimension
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = (metaffi_uint8(1));
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = (metaffi_uint8(2));

		pcdts[0].cdt_val.array_val->arr[1].set_new_array(3, 1, metaffi_uint8_type);// array of length 2 and 1 fixed dimension
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = (metaffi_uint8(3));
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = (metaffi_uint8(4));
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2] = (metaffi_uint8(5));

		pcdts[0].cdt_val.array_val->arr[2].set_new_array(4, 1, metaffi_uint8_type);// array of length 2 and 1 fixed dimension
		pcdts[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0] = (metaffi_uint8(6));
		pcdts[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[1] = (metaffi_uint8(7));
		pcdts[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[2] = (metaffi_uint8(8));
		pcdts[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[3] = (metaffi_uint8(9));

		cdts_java_wrapper cdts_j(&pcdts);
		jvalue jval = cdts_j.to_jvalue(env, 0);

		jarray_wrapper jarr(env, (jobjectArray) jval.l, metaffi_array_type);

		REQUIRE(((jarr.size() == 3)));

		jbyte* arr0 = env->GetByteArrayElements((jbyteArray) jarr.get(0).first.l, nullptr);
		jbyte* arr1 = env->GetByteArrayElements((jbyteArray) jarr.get(1).first.l, nullptr);
		jbyte* arr2 = env->GetByteArrayElements((jbyteArray) jarr.get(2).first.l, nullptr);

		REQUIRE((env->GetArrayLength((jbyteArray) jarr.get(0).first.l) == 2));
		REQUIRE((arr0[0] == 1));
		REQUIRE((arr0[1] == 2));

		REQUIRE((env->GetArrayLength((jbyteArray) jarr.get(1).first.l) == 3));
		REQUIRE((arr1[0] == 3));
		REQUIRE((arr1[1] == 4));
		REQUIRE((arr1[2] == 5));

		REQUIRE((env->GetArrayLength((jbyteArray) jarr.get(2).first.l) == 4));
		REQUIRE((arr2[0] == 6));
		REQUIRE((arr2[1] == 7));
		REQUIRE((arr2[2] == 8));
		REQUIRE((arr2[3] == 9));

		env->ReleaseByteArrayElements((jbyteArray) jarr.get(0).first.l, arr0, 0);
		env->ReleaseByteArrayElements((jbyteArray) jarr.get(1).first.l, arr1, 0);
		env->ReleaseByteArrayElements((jbyteArray) jarr.get(2).first.l, arr2, 0);
	}


	TEST_CASE("Traverse 1D bytes array")
	{
		cdts pcdts(1, 1);                        // array of length 1 (i.e. one parameter) and 1 fixed dimension
		pcdts[0].set_new_array(3, 1, metaffi_uint8_type);// array of length 3 and 1 fixed dimensions

		pcdts[0].cdt_val.array_val->arr[0] = (metaffi_uint8(1));
		pcdts[0].cdt_val.array_val->arr[1] = (metaffi_uint8(2));
		pcdts[0].cdt_val.array_val->arr[2] = (metaffi_uint8(3));

		cdts_java_wrapper cdts_j(&pcdts);
		jvalue jval = cdts_j.to_jvalue(env, 0);

		jarray_wrapper jarr(env, (jarray) jval.l, metaffi_array_type);
		REQUIRE((jarr.size() == 3));

		jbyte* bytes = env->GetByteArrayElements((jbyteArray) jval.l, nullptr);

		REQUIRE(((metaffi_uint8) bytes[0] == 1));
		REQUIRE(((metaffi_uint8) bytes[1] == 2));
		REQUIRE(((metaffi_uint8) bytes[2] == 3));

		env->ReleaseByteArrayElements((jbyteArray) jval.l, bytes, 0);
	}

	TEST_CASE("Traverse 1D float array")
	{
		cdts pcdts(1, 1);
		pcdts[0].set_new_array(3, 1, metaffi_float32_type);
		pcdts[0].cdt_val.array_val->arr[0] = (metaffi_float32(1.0f));
		pcdts[0].cdt_val.array_val->arr[1] = (metaffi_float32(2.0f));
		pcdts[0].cdt_val.array_val->arr[2] = (metaffi_float32(3.0f));

		cdts_java_wrapper cdts_j(&pcdts);
		jvalue jval = cdts_j.to_jvalue(env, 0);

		REQUIRE((env->GetArrayLength((jarray) jval.l) == 3));
		jfloat* floats = env->GetFloatArrayElements((jfloatArray) jval.l, nullptr);

		REQUIRE(((metaffi_float32) floats[0] == 1.0f));
		REQUIRE(((metaffi_float32) floats[1] == 2.0f));
		REQUIRE(((metaffi_float32) floats[2] == 3.0f));

		env->ReleaseFloatArrayElements((jfloatArray) jval.l, floats, 0);
	}


	TEST_CASE("Traverse 3D array")
	{
		cdts pcdts(1, 1);                          // one parameter
		pcdts[0].set_new_array(2, 3, metaffi_float32_type);// []

		pcdts[0].cdt_val.array_val->arr[0].set_new_array(2, 2, metaffi_float32_type);// [0][]
		pcdts[0].cdt_val.array_val->arr[1].set_new_array(3, 2, metaffi_float32_type);// [1][]

		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].set_new_array(4, 1, metaffi_float32_type);// [0][0][]
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].set_new_array(2, 1, metaffi_float32_type);// [0][1][]

		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].set_new_array(3, 1, metaffi_float32_type);// [1][0][]
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].set_new_array(2, 1, metaffi_float32_type);// [1][1][]
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].set_new_array(1, 1, metaffi_float32_type);// [1][2][]

		// [0][0][] = {1.0f, 2.0f, 3.0f, 4.0f}
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = (metaffi_float32(1.0f));
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = (metaffi_float32(2.0f));
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = (metaffi_float32(3.0f));
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[3] = (metaffi_float32(4.0f));

		// [0][1][] = {5.0f, 6.0f}
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = (metaffi_float32(5.0f));
		pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = (metaffi_float32(6.0f));

		// [1][0][] = {7.0f, 8.0f, 9.0f}
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = (metaffi_float32(7.0f));
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = (metaffi_float32(8.0f));
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = (metaffi_float32(9.0f));

		// [1][1][] = {10.0f, 11.0f}
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = (metaffi_float32(10.0f));
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = (metaffi_float32(11.0f));

		// [1][2][] = {12.0f}
		pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0] = (metaffi_float32(12.0f));

		cdts_java_wrapper cdts_j(&pcdts);
		jvalue jval = cdts_j.to_jvalue(env, 0);

		jarray_wrapper jarr(env, (jobjectArray) jval.l, metaffi_array_type);
		REQUIRE((jarr.size() == 2));

		jarray_wrapper jarr2dim1(env, (jobjectArray) jarr.get(0).first.l, metaffi_array_type);
		jarray_wrapper jarr2dim2(env, (jobjectArray) jarr.get(1).first.l, metaffi_array_type);

		REQUIRE((jarr2dim1.size() == 2));
		REQUIRE((jarr2dim2.size() == 3));

		jfloatArray jarr1dim00 = (jfloatArray) jarr2dim1.get(0).first.l;
		jfloatArray jarr1dim01 = (jfloatArray) jarr2dim1.get(1).first.l;

		jfloatArray jarr1dim10 = (jfloatArray) jarr2dim2.get(0).first.l;
		jfloatArray jarr1dim11 = (jfloatArray) jarr2dim2.get(1).first.l;
		jfloatArray jarr1dim12 = (jfloatArray) jarr2dim2.get(2).first.l;

		REQUIRE((env->GetArrayLength(jarr1dim00) == 4));
		REQUIRE((env->GetArrayLength(jarr1dim01) == 2));
		REQUIRE((env->GetArrayLength(jarr1dim10) == 3));
		REQUIRE((env->GetArrayLength(jarr1dim11) == 2));
		REQUIRE((env->GetArrayLength(jarr1dim12) == 1));

		jfloat* lst1dim00 = env->GetFloatArrayElements(jarr1dim00, nullptr);
		jfloat* lst1dim01 = env->GetFloatArrayElements(jarr1dim01, nullptr);
		jfloat* lst1dim10 = env->GetFloatArrayElements(jarr1dim10, nullptr);
		jfloat* lst1dim11 = env->GetFloatArrayElements(jarr1dim11, nullptr);
		jfloat* lst1dim12 = env->GetFloatArrayElements(jarr1dim12, nullptr);

		REQUIRE(((metaffi_float32) lst1dim00[0] == 1.0f));
		REQUIRE(((metaffi_float32) lst1dim00[1] == 2.0f));
		REQUIRE(((metaffi_float32) lst1dim00[2] == 3.0f));
		REQUIRE(((metaffi_float32) lst1dim00[3] == 4.0f));
		REQUIRE(((metaffi_float32) lst1dim01[0] == 5.0f));
		REQUIRE(((metaffi_float32) lst1dim01[1] == 6.0f));
		REQUIRE(((metaffi_float32) lst1dim10[0] == 7.0f));
		REQUIRE(((metaffi_float32) lst1dim10[1] == 8.0f));
		REQUIRE(((metaffi_float32) lst1dim10[2] == 9.0f));
		REQUIRE(((metaffi_float32) lst1dim11[0] == 10.0f));
		REQUIRE(((metaffi_float32) lst1dim11[1] == 11.0f));
		REQUIRE(((metaffi_float32) lst1dim12[0] == 12.0f));
	}

	TEST_CASE("Construct 1D bytes array")// TODO: check with "-1" dimensions
	{
		uint8_t arr[] = {1, 2, 3};
		jbyteArray jarr = env->NewByteArray(3);
		env->SetByteArrayRegion(jarr, 0, 3, (jbyte*) arr);

		cdts pcdts(1, 1);// one argument
		cdts_java_wrapper jcdts(&pcdts);

		metaffi_type_info info = metaffi_type_info{metaffi_uint8_array_type, nullptr, false, 1};
		jvalue cdt_val{.l = jarr};
		jcdts.from_jvalue(env, cdt_val, 'L', info, 0);

		REQUIRE((pcdts.fixed_dimensions == 1));
		REQUIRE((pcdts.length == 1));
		REQUIRE((pcdts[0].type == metaffi_uint8_array_type));
		REQUIRE((pcdts[0].free_required != 0));
		REQUIRE((pcdts[0].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->length == 3));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.uint8_val == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.uint8_val == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[2].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[2].cdt_val.uint8_val == 3));
	}

	TEST_CASE("Construct 2D bytes array")
	{
		uint8_t arr1[] = {1, 2, 3};
		jbyteArray jarr1 = env->NewByteArray(3);
		env->SetByteArrayRegion(jarr1, 0, 3, (jbyte*) arr1);

		uint8_t arr2[] = {4, 5, 6};
		jbyteArray jarr2 = env->NewByteArray(3);
		env->SetByteArrayRegion(jarr2, 0, 3, (jbyte*) arr2);

		// create byte[][] array and set arr1 and arr2
		jobjectArray jarr = env->NewObjectArray(2, env->FindClass("[B"), nullptr);
		env->SetObjectArrayElement(jarr, 0, jarr1);
		env->SetObjectArrayElement(jarr, 1, jarr2);

		cdts pcdts(1, 1);// one argument
		cdts_java_wrapper jcdts(&pcdts);

		metaffi_type_info info = metaffi_type_info{metaffi_uint8_array_type, nullptr, false, 2};
		jvalue cdt_val{.l = jarr};
		jcdts.from_jvalue(env, cdt_val, 'L', info, 0);

		REQUIRE((pcdts[0].cdt_val.array_val->fixed_dimensions == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->length == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->length == 3));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->length == 3));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.uint8_val == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.uint8_val == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2].cdt_val.uint8_val == 3));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.uint8_val == 4));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.uint8_val == 5));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].type == metaffi_uint8_type));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.uint8_val == 6));
	}

	TEST_CASE("Construct 1D array")
	{
		jfloatArray jarr = env->NewFloatArray(3);
		jfloat arr[] = {1.0f, 2.0f, 3.0f};
		env->SetFloatArrayRegion(jarr, 0, 3, arr);

		cdts pcdts(1, 1);
		pcdts[0].type = metaffi_float32_array_type;

		cdts_java_wrapper jcdts(&pcdts);
		metaffi_type_info info = metaffi_type_info{metaffi_float32_array_type, nullptr, false, 1};
		jvalue jval;
		jval.l = (jobject) jarr;
		jcdts.from_jvalue(env, jval, 'L', info, 0);

		REQUIRE((pcdts[0].type == metaffi_float32_array_type));
		REQUIRE((pcdts[0].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->length == 3));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.float32_val == 1.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.float32_val == 2.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[2].cdt_val.float32_val == 3.0f));
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

		auto jarr2dim0 = env->NewObjectArray(2, env->FindClass("[F"), nullptr);
		env->SetObjectArrayElement(jarr2dim0, 0, jarr1dim00);
		env->SetObjectArrayElement(jarr2dim0, 1, jarr1dim01);

		auto jarr2dim1 = env->NewObjectArray(3, env->FindClass("[F"), nullptr);
		env->SetObjectArrayElement(jarr2dim1, 0, jarr1dim10);
		env->SetObjectArrayElement(jarr2dim1, 1, jarr1dim11);
		env->SetObjectArrayElement(jarr2dim1, 2, jarr1dim12);

		auto jarr = env->NewObjectArray(2, env->FindClass("[[F"), nullptr);
		env->SetObjectArrayElement(jarr, 0, (jobject) jarr2dim0);
		env->SetObjectArrayElement(jarr, 1, (jobject) jarr2dim1);

		cdts pcdts(1, 1);
		cdts_java_wrapper jcdts(&pcdts);

		metaffi_type_info info = metaffi_type_info{metaffi_float32_array_type, nullptr, false, 3};
		jvalue cdt_val;
		cdt_val.l = (jobject) jarr;
		jcdts.from_jvalue(env, cdt_val, 'L', info, 0);

		REQUIRE((pcdts[0].cdt_val.array_val->fixed_dimensions == 3));
		REQUIRE((pcdts[0].cdt_val.array_val->length == 2));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->fixed_dimensions == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->length == 2));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->length == 4));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.float32_val == 1.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.float32_val == 2.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2].cdt_val.float32_val == 3.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[3].cdt_val.float32_val == 4.0f));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->length == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.float32_val == 5.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.float32_val == 6.0f));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->fixed_dimensions == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->length == 3));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->length == 3));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.float32_val == 7.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.float32_val == 8.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2].cdt_val.float32_val == 9.0f));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->length == 2));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.float32_val == 10.0f));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.float32_val == 11.0f));

		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.array_val->length == 1));
		REQUIRE((pcdts[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0].cdt_val.float32_val == 12.0f));
	}

	TEST_CASE("!create_new_object")
	{
		// TODO
	}
}