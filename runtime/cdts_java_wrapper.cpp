#include "cdts_java_wrapper.h"
#include <algorithm>
#include <utility>
#include "runtime_id.h"
#include "jni_metaffi_handle.h"
#include "exception_macro.h"
#include "utils/tracer.h"
#include "jni_caller.h"

template<typename jni_primitive_t, typename jni_primitive_array_t>
std::pair<jni_primitive_t*, metaffi_size*> copy_jvm_array(JNIEnv* env, bool is_array_of_objects, void* getter_func, jmethodID methID, jobjectArray obj_array, int dimensions)
{
	// Handle multi-dimensional array
	if(dimensions == 1)
	{
		metaffi_size len = env->GetArrayLength(obj_array);
		auto arr = new jni_primitive_t[len];
	
		if(is_array_of_objects)
		{
			for (int i = 0; i < len; i++)
			{
				jobject obj = env->GetObjectArrayElement(obj_array, i);
				check_and_throw_jvm_exception(env, true);
				arr[i] = ((jni_primitive_t(*)(JNIEnv*, jobject, jmethodID))getter_func)(env, obj, methID); // TODO: need to use the correct function
				check_and_throw_jvm_exception(env, true);
				env->DeleteLocalRef(obj);
			}
		}
		else
		{
			jni_primitive_t* data = ((jni_primitive_t*(*)(JNIEnv*,jni_primitive_array_t, jboolean*))getter_func)(env, (jni_primitive_array_t)obj_array, nullptr);
			check_and_throw_jvm_exception(env, true);
			std::copy(data, data+len, arr);
		}
		
		return std::pair<jni_primitive_t*, metaffi_size*>(arr, (metaffi_size*)len);
	}
	else
	{
		auto len = env->GetArrayLength(obj_array);
		auto arr = new jni_primitive_t[len];
		auto plen = new metaffi_size[len];
		
		for (jsize i = 0; i < len; i++)
		{
			jobjectArray innerArray = (jobjectArray)env->GetObjectArrayElement(obj_array, i);
			auto res = copy_jvm_array<jni_primitive_t, jni_primitive_array_t>(env, is_array_of_objects, getter_func, methID, innerArray, dimensions - 1);
			((jni_primitive_t**)arr)[i] = res.first;
			plen[i] = (metaffi_size)res.second;
			env->DeleteLocalRef(innerArray);
		}
		
		return std::pair<jni_primitive_t*, metaffi_size*>(arr, plen);
	}
}

#define copy_jni_array(name_of_java_class_type, jni_sig, name_of_getter, cdt_type_struct, jni_primitive_type, ctype, dims) \
    jclass objectClass = env->GetObjectClass(val.l); \
	jmethodID getClassMethod = env->GetMethodID(objectClass, "getClass", "()Ljava/lang/Class;"); \
	jobject classObject = env->CallObjectMethod(val.l, getClassMethod);                                                                                                                    \
	jclass classClass = env->GetObjectClass(classObject);                                                                                                                    \
	jmethodID getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");                          \
	jstring className = (jstring)env->CallObjectMethod(classObject, getNameMethod);\
    const char* cClassName = env->GetStringUTFChars(className, nullptr);                                                                                                                \
    bool is_array_of_objects = (strstr(cClassName, "/java/lang/") != nullptr);                                                                                                                \
    env->ReleaseStringUTFChars(className, cClassName);                                                                          \
    check_and_throw_jvm_exception(env, true);\
                                                                                                                    \
    cdt_type_struct.dimensions = dims; \
    if(is_array_of_objects)                                                                                         \
	{                                                                                                                  \
		jclass cls = env->FindClass("Ljava/lang/" #name_of_java_class_type ";");\
		jmethodID methID = env->GetMethodID(cls, name_of_getter, "()" jni_sig);\
		int len = env->GetArrayLength((jobjectArray)val.l);\
		check_and_throw_jvm_exception(env, true);\
		\
	    auto res = copy_jvm_array<jni_primitive_type, jni_primitive_type##Array>(env, is_array_of_objects, (void*)env->functions->Call##name_of_java_class_type##Method, methID, (jobjectArray)val.l, dims); \
		cdt_type_struct.dimensions_lengths = res.second;\
		cdt_type_struct.vals = (ctype*)res.first; \
	}   \
	else  \
	{  \
		auto res = copy_jvm_array<jni_primitive_type, jni_primitive_type##Array>(env, is_array_of_objects, (void*)env->functions->Get##name_of_java_class_type##ArrayElements, nullptr, (jobjectArray)val.l, dims); \
		cdt_type_struct.dimensions_lengths = res.second;\
		cdt_type_struct.vals = (ctype*)res.first; \
	}
	
template<typename jni_primitive_t, typename jni_array_primitive_t, typename c_primitive_t>
jobjectArray create_jvm_multidim_array(JNIEnv *env, c_primitive_t* arr, uint64_t dims, uint64_t* lengths, void* new_array_func, void* set_array_region_func, const char* jni_arr_sig, uint64_t cur_dim = 0)
{
	if (cur_dim+1 == dims)
	{
		jni_array_primitive_t data_arr = ((jni_array_primitive_t(*)(JNIEnv*, jsize))new_array_func)(env, (jsize)lengths[cur_dim]);
		((void(*)(JNIEnv*,jni_array_primitive_t,jsize,jsize,jni_primitive_t*))set_array_region_func)(env,data_arr, 0, (jsize)lengths[cur_dim], (jni_primitive_t*)arr);
		return (jobjectArray)data_arr;
	}
	else if (cur_dim+1 < dims)
	{
		jobjectArray outerArray = env->NewObjectArray((jsize)lengths[cur_dim], env->FindClass(jni_arr_sig), nullptr);
		
		for (jsize i = 0; i < lengths[cur_dim]; i++)
		{
			env->SetObjectArrayElement(outerArray, i, create_jvm_multidim_array<jni_primitive_t, jni_array_primitive_t, c_primitive_t>(env, arr + i * lengths[cur_dim], dims, lengths, new_array_func, set_array_region_func, jni_arr_sig, cur_dim + 1));
		}
		return outerArray;
	}
	return nullptr;
}

jobjectArray create_jvm_multidim_array(JNIEnv *env, cdt_metaffi_handle* arr, uint64_t dims, uint64_t* lengths, uint64_t cur_dim = 0)
{
	if (cur_dim+1 == dims)
	{
		jobjectArray data_arr = env->NewObjectArray((jsize)lengths[cur_dim], env->FindClass("java/lang/Object"), nullptr);
		
		for (jsize i = 0; i < lengths[0]; i++)
		{
			env->SetObjectArrayElement(data_arr, i, reinterpret_cast<jobject>(arr[i].val));
		}
		
		return data_arr;
	}
	else if (cur_dim+1 < dims)
	{
		jobjectArray outerArray = env->NewObjectArray((jsize)lengths[cur_dim], env->FindClass("[Ljava/lang/Object;"), nullptr);
		
		for (jsize i = 0; i < lengths[cur_dim]; i++)
		{
			env->SetObjectArrayElement(outerArray, i, create_jvm_multidim_array(env, arr + i * lengths[cur_dim], dims, lengths,cur_dim + 1));
		}
		return outerArray;
	}
	return nullptr;
}

//--------------------------------------------------------------------
cdts_java_wrapper::cdts_java_wrapper(cdt *cdts, metaffi_size cdts_length):metaffi::runtime::cdts_wrapper(cdts, cdts_length, false)
{
}
//--------------------------------------------------------------------
jvalue cdts_java_wrapper::to_jvalue(JNIEnv* env, int index) const
{
	jvalue jval;
	cdt* c = (*this)[index];

	switch (c->type)
	{
		case metaffi_bool_type:
			jval.z = c->cdt_val.metaffi_bool_val.val ? JNI_TRUE : JNI_FALSE;
			break;
		case metaffi_int32_type:
			jval.i = c->cdt_val.metaffi_int32_val.val;
			break;
		case metaffi_int64_type:
			jval.j = c->cdt_val.metaffi_int64_val.val;
			break;
		case metaffi_int16_type:
			jval.s = c->cdt_val.metaffi_int16_val.val;
			break;
		case metaffi_int8_type:
			jval.b = c->cdt_val.metaffi_int8_val.val;
			break;
		case metaffi_float32_type:
			jval.f = c->cdt_val.metaffi_float32_val.val;
			break;
		case metaffi_float64_type:
			jval.d = c->cdt_val.metaffi_float64_val.val;
			break;
		case metaffi_handle_type:
			// if runtime_id is NOT openjdk, wrap in MetaFFIHandle
			if(c->cdt_val.metaffi_handle_val.runtime_id != OPENJDK_RUNTIME_ID)
			{
				jni_metaffi_handle h(env, c->cdt_val.metaffi_handle_val.val, c->cdt_val.metaffi_handle_val.runtime_id);
				jval.l = h.new_jvm_object(env);
			}
			else
			{
				jval.l = static_cast<jobject>(c->cdt_val.metaffi_handle_val.val);
			}
			break;
		case metaffi_string8_type:
		{
			jstring str = env->NewStringUTF(std::string(c->cdt_val.metaffi_string8_val.val,
			                                            c->cdt_val.metaffi_string8_val.length).c_str());
			check_and_throw_jvm_exception(env, true);
			jval.l = str;
		}break;
		case metaffi_string16_type:
		{
			jstring str = env->NewString((jchar*)c->cdt_val.metaffi_string16_val.val,
			                                            c->cdt_val.metaffi_string16_val.length);
			check_and_throw_jvm_exception(env, true);
			jval.l = str;
		}break;
		
		// Add cases for array types
		case metaffi_bool_array_type:
		{
			jval.l = create_jvm_multidim_array<jboolean, jbooleanArray, metaffi_bool>(env,
			                                                             c->cdt_val.metaffi_bool_array_val.vals,
			                                                             c->cdt_val.metaffi_bool_array_val.dimensions,
			                                                             c->cdt_val.metaffi_bool_array_val.dimensions_lengths,
			                                                             (void*)env->functions->NewBooleanArray,
			                                                             (void*)env->functions->SetBooleanArrayRegion,
			                                                             "[Z");
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_int32_array_type:
		{
			jval.l = create_jvm_multidim_array<jint, jintArray, int32_t>(env,
			                                                               c->cdt_val.metaffi_int32_array_val.vals,
			                                                               c->cdt_val.metaffi_int32_array_val.dimensions,
			                                                               c->cdt_val.metaffi_int32_array_val.dimensions_lengths,
			                                                               (void*)env->functions->NewIntArray,
			                                                               (void*)env->functions->SetIntArrayRegion,
			                                                               "[I");
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_int64_array_type:
		{
			jval.l = create_jvm_multidim_array<jlong, jlongArray, int64_t>(env,
			                                                             c->cdt_val.metaffi_int64_array_val.vals,
			                                                             c->cdt_val.metaffi_int64_array_val.dimensions,
			                                                             c->cdt_val.metaffi_int64_array_val.dimensions_lengths,
			                                                             (void*)env->functions->NewLongArray,
			                                                             (void*)env->functions->SetLongArrayRegion,
			                                                             "[L");
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_int16_array_type:
		{
			jval.l = create_jvm_multidim_array<jshort, jshortArray, int16_t>(env,
			                                                               c->cdt_val.metaffi_int16_array_val.vals,
			                                                               c->cdt_val.metaffi_int16_array_val.dimensions,
			                                                               c->cdt_val.metaffi_int16_array_val.dimensions_lengths,
			                                                               (void*)env->functions->NewShortArray,
			                                                               (void*)env->functions->SetShortArrayRegion,
			                                                               "[S");
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_uint8_array_type:
		{
			jval.l = create_jvm_multidim_array<jbyte, jbyteArray, uint8_t>(env,
																		   c->cdt_val.metaffi_uint8_array_val.vals,
																		   c->cdt_val.metaffi_uint8_array_val.dimensions,
																		   c->cdt_val.metaffi_uint8_array_val.dimensions_lengths,
																		   (void*)env->functions->NewByteArray,
																		   (void*)env->functions->SetByteArrayRegion,
																		   "[B");
			check_and_throw_jvm_exception(env, true);
			
		} break;
		case metaffi_int8_array_type:
		{
			jval.l = create_jvm_multidim_array<jbyte, jbyteArray, int8_t>(env,
			                                                               c->cdt_val.metaffi_int8_array_val.vals,
			                                                               c->cdt_val.metaffi_int8_array_val.dimensions,
			                                                               c->cdt_val.metaffi_int8_array_val.dimensions_lengths,
			                                                               (void*)env->functions->NewByteArray,
			                                                               (void*)env->functions->SetByteArrayRegion,
			                                                               "[B");
			check_and_throw_jvm_exception(env, true);
			
		} break;
		case metaffi_float32_array_type:
		{
			jval.l = create_jvm_multidim_array<jfloat, jfloatArray, float>(env,
		                                                                 c->cdt_val.metaffi_float32_array_val.vals,
		                                                                 c->cdt_val.metaffi_float32_array_val.dimensions,
		                                                                 c->cdt_val.metaffi_float32_array_val.dimensions_lengths,
		                                                                 (void*)env->functions->NewFloatArray,
		                                                                 (void*)env->functions->SetFloatArrayRegion,
		                                                                 "[F");
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_float64_array_type:
		{
			jval.l = create_jvm_multidim_array<jdouble, jdoubleArray, double>(env,
			                                                               c->cdt_val.metaffi_float64_array_val.vals,
			                                                               c->cdt_val.metaffi_float64_array_val.dimensions,
			                                                               c->cdt_val.metaffi_float64_array_val.dimensions_lengths,
			                                                               (void*)env->functions->NewDoubleArray,
			                                                               (void*)env->functions->SetDoubleArrayRegion,
			                                                               "[D");
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_handle_array_type:
		{
			jval.l = create_jvm_multidim_array(env,
									  c->cdt_val.metaffi_handle_array_val.vals,
									  c->cdt_val.metaffi_handle_array_val.dimensions,
									  c->cdt_val.metaffi_handle_array_val.dimensions_lengths);
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_string8_array_type:
		{
			if (c->cdt_val.metaffi_string8_array_val.dimensions != 1)
			{
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jobjectArray arr = env->NewObjectArray(c->cdt_val.metaffi_string8_array_val.dimensions_lengths[0], env->FindClass("java/lang/String"), nullptr);
			check_and_throw_jvm_exception(env, true);
			for (int i = 0; i < c->cdt_val.metaffi_string8_array_val.dimensions_lengths[0]; i++)
			{
				jstring str = env->NewStringUTF(std::string(c->cdt_val.metaffi_string8_array_val.vals[i], c->cdt_val.metaffi_string8_array_val.vals_sizes[i]).c_str());
				check_and_throw_jvm_exception(env, true);
				env->SetObjectArrayElement(arr, i, str);
				check_and_throw_jvm_exception(env, true);
				env->DeleteLocalRef(str);
				check_and_throw_jvm_exception(env, true);
			}
			
			jval.l = arr;
		}break;
		case metaffi_string16_array_type:
		{
			if (c->cdt_val.metaffi_string16_array_val.dimensions != 1)
			{
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jobjectArray arr = env->NewObjectArray(c->cdt_val.metaffi_string16_array_val.dimensions_lengths[0], env->FindClass("java/lang/String"), nullptr);
			check_and_throw_jvm_exception(env, true);
			for (int i = 0; i < c->cdt_val.metaffi_string16_array_val.dimensions_lengths[0]; i++)
			{
				jstring str = env->NewString((const jchar*)c->cdt_val.metaffi_string16_array_val.vals[i], c->cdt_val.metaffi_string16_array_val.vals_sizes[i]);
				check_and_throw_jvm_exception(env, true);
				env->SetObjectArrayElement(arr, i, str);
				check_and_throw_jvm_exception(env, true);
				env->DeleteLocalRef(str);
				check_and_throw_jvm_exception(env, true);
			}
			
			jval.l = arr;
		}break;

		case metaffi_callable_type:
		{
			// return LoadCallable.CallableWithArgs instance by calling LoadCallable.load()

			jclass load_callable_cls = env->FindClass("metaffi/Caller");
			check_and_throw_jvm_exception(env, true);

			jmethodID create_caller = env->GetStaticMethodID(load_callable_cls, "createCaller", "(J[J[J)Lmetaffi/Caller;");
			check_and_throw_jvm_exception(env, true);

			// Convert parameters_types to Java long[]
			jlongArray jParametersTypesArray = env->NewLongArray(c->cdt_val.metaffi_callable_val.params_types_length);
			env->SetLongArrayRegion(jParametersTypesArray, 0, c->cdt_val.metaffi_callable_val.params_types_length, (const jlong*)c->cdt_val.metaffi_callable_val.parameters_types);

			// Convert retval_types to Java long[]
			jlongArray jRetvalsTypesArray = env->NewLongArray(c->cdt_val.metaffi_callable_val.retval_types_length);
			env->SetLongArrayRegion(jRetvalsTypesArray, 0, c->cdt_val.metaffi_callable_val.retval_types_length, (const jlong*)c->cdt_val.metaffi_callable_val.retval_types);


			jval.l = env->CallStaticObjectMethod(load_callable_cls, create_caller,
												c->cdt_val.metaffi_callable_val.val,
												jParametersTypesArray,
												jRetvalsTypesArray);
			check_and_throw_jvm_exception(env, true);

			if(env->GetObjectRefType(jParametersTypesArray) == JNILocalRefType)
			{
				env->DeleteLocalRef(jParametersTypesArray);
			}

			if(env->GetObjectRefType(jRetvalsTypesArray) == JNILocalRefType)
			{
				env->DeleteLocalRef(jRetvalsTypesArray);
			}

		}break;

		default:
			std::stringstream ss;
			ss << "Unsupported type to convert to Java: " << c->type;
			throw std::runtime_error(ss.str());
	}
	
	return jval;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::from_jvalue(JNIEnv* env, jvalue val, const metaffi_type_info& type, int index) const
{
	cdt* c = (*this)[index];
	c->type = type.type;
	switch (c->type)
	{
		case metaffi_int32_type:
			c->cdt_val.metaffi_int32_val.val = val.i;
			c->type = metaffi_int32_type;
			break;
		case metaffi_int64_type:
			c->cdt_val.metaffi_int64_val.val = val.j;
			c->type = metaffi_int64_type;
			break;
		case metaffi_int16_type:
			c->cdt_val.metaffi_int16_val.val = val.s;
			c->type = metaffi_int16_type;
			break;
		case metaffi_int8_type:
			c->cdt_val.metaffi_int8_val.val = val.b;
			c->type = metaffi_int8_type;
			break;
		case metaffi_float32_type:
			c->cdt_val.metaffi_float32_val.val = val.f;
			c->type = metaffi_float32_type;
			break;
		case metaffi_float64_type:
			c->cdt_val.metaffi_float64_val.val = val.d;
			c->type = metaffi_float64_type;
			break;
		case metaffi_handle_type:
			// if val.l is MetaFFIHandle - get its inner data
			if(jni_metaffi_handle::is_metaffi_handle_wrapper_object(env, val.l))
			{
				jni_metaffi_handle h(env, val.l);
				c->cdt_val.metaffi_handle_val.val = h.get_handle();
				c->cdt_val.metaffi_handle_val.runtime_id = h.get_runtime_id(); // mark as openjdk object
				c->type = metaffi_handle_type;
			}
			else
			{
				c->cdt_val.metaffi_handle_val.val = env->NewGlobalRef(val.l);
				c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID; // mark as openjdk object
				c->type = metaffi_handle_type;
			}
			break;
		case metaffi_callable_type:
			// get from val.l should be Caller class
			if(!is_caller_class(env, val.l))
			{
				throw std::runtime_error("caller_type expects metaffi.Caller object");
			}
			fill_callable_cdt(env, val.l, c->cdt_val.metaffi_callable_val.val,
								c->cdt_val.metaffi_callable_val.parameters_types, c->cdt_val.metaffi_callable_val.params_types_length,
								c->cdt_val.metaffi_callable_val.retval_types, c->cdt_val.metaffi_callable_val.retval_types_length);
			c->type = metaffi_callable_type;
			break;
		case metaffi_bool_type:
			c->cdt_val.metaffi_bool_val.val = val.z;
			c->type = metaffi_bool_type;
			break;
		case metaffi_string8_type:
		{
			jstring str = static_cast<jstring>(val.l);
			const char* chars = env->GetStringUTFChars(str, nullptr);
			check_and_throw_jvm_exception(env, true);
			int len = env->GetStringUTFLength(str);
			check_and_throw_jvm_exception(env, true);
			c->type = metaffi_string8_type;
			c->cdt_val.metaffi_string8_val.val = new char[len]; // You might need to copy the string
			c->cdt_val.metaffi_string8_val.length = len;
			std::copy_n(chars, len, c->cdt_val.metaffi_string8_val.val);
			env->ReleaseStringUTFChars(str, chars);
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_string8_array_type:
		{
			
			c->type = metaffi_string8_array_type;
			jobjectArray arr = static_cast<jobjectArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			
			c->cdt_val.metaffi_string8_array_val.dimensions = 1;
			c->cdt_val.metaffi_string8_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_string8_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_string8_array_val.vals = new metaffi_string8[len];
			c->cdt_val.metaffi_string8_array_val.vals_sizes = new metaffi_size[len];
			for (int i = 0; i < len; ++i)
			{
				
				jstring str = static_cast<jstring>(env->GetObjectArrayElement(arr, i));
				check_and_throw_jvm_exception(env, true);
				
				const char* chars = env->GetStringUTFChars(str, nullptr);
				check_and_throw_jvm_exception(env, true);
				int strlen = env->GetStringUTFLength(str);
				check_and_throw_jvm_exception(env, true);
				
				c->cdt_val.metaffi_string8_array_val.vals[i] = new char[len+1]; // You might need to copy the string
				c->cdt_val.metaffi_string8_array_val.vals_sizes[i] = strlen;
				
				std::copy(chars, chars+strlen, c->cdt_val.metaffi_string8_array_val.vals[i]);
				
				c->cdt_val.metaffi_string8_array_val.vals[i][strlen] = '\0';
				
				env->ReleaseStringUTFChars(str, chars);
				
				check_and_throw_jvm_exception(env, true);
			}
		} break;
		case metaffi_int64_array_type:
		{
			c->type = metaffi_int64_array_type;
			copy_jni_array(Long, "J", "longValue", c->cdt_val.metaffi_int64_array_val, jlong, metaffi_int64, type.dimensions);

		} break;
		case metaffi_int16_array_type:
		{
			c->type = metaffi_int16_array_type;
			copy_jni_array(Short, "S", "shortValue", c->cdt_val.metaffi_int16_array_val, jshort, metaffi_int16, type.dimensions);
		} break;
		case metaffi_uint8_array_type:
		{
			c->type = metaffi_uint8_array_type;
			copy_jni_array(Byte, "B", "byteValue", c->cdt_val.metaffi_uint8_array_val, jbyte, metaffi_uint8, type.dimensions);
		} break;
		case metaffi_int8_array_type:
		{
			c->type = metaffi_int8_array_type;
			copy_jni_array(Byte, "B", "byteValue", c->cdt_val.metaffi_int8_array_val, jbyte, metaffi_int8,type.dimensions);
		} break;
		case metaffi_bool_array_type:
		{
			c->type = metaffi_bool_array_type;
			copy_jni_array(Boolean, "Z", "booleanValue", c->cdt_val.metaffi_bool_array_val, jboolean, metaffi_bool, type.dimensions);
		} break;
		case metaffi_float32_array_type:
		{
			c->type = metaffi_float32_array_type;
			copy_jni_array(Float, "F", "floatValue", c->cdt_val.metaffi_float32_array_val, jfloat, metaffi_float32, type.dimensions);
		} break;
		case metaffi_float64_array_type:
		{
			c->type = metaffi_float64_array_type;
			copy_jni_array(Double, "D", "doubleValue", c->cdt_val.metaffi_float64_array_val, jdouble, metaffi_float64, type.dimensions);
		} break;
		case metaffi_handle_array_type:
		{
			c->type = metaffi_handle_array_type;
			jobjectArray arr = static_cast<jobjectArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			c->cdt_val.metaffi_handle_array_val.dimensions = 1;
			c->cdt_val.metaffi_handle_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_handle_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_handle_array_val.vals = new cdt_metaffi_handle[len];
			for (int i = 0; i < len; ++i)
			{
				c->cdt_val.metaffi_handle_array_val.vals[i].val = env->GetObjectArrayElement(arr, i);
				c->cdt_val.metaffi_handle_array_val.vals[i].runtime_id = OPENJDK_RUNTIME_ID;
			}
		} break;
		case metaffi_any_type:
		{
			// returned type is an object, we need to *try* and switch it to primitive if it
			// can be converted to primitive
			c->cdt_val.metaffi_handle_val.val = val.l;
			c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
			c->type = metaffi_handle_type;

			switch_to_primitive(env, index);
			
			if(c->type == metaffi_handle_type)
			{
				if(jni_metaffi_handle::is_metaffi_handle_wrapper_object(env, val.l))
				{
					jni_metaffi_handle h(env, val.l);
					c->cdt_val.metaffi_handle_val.val = h.get_handle();
					c->cdt_val.metaffi_handle_val.runtime_id = h.get_runtime_id(); // mark as openjdk object
					c->type = metaffi_handle_type;
				}
				else
				{
					c->cdt_val.metaffi_handle_val.val = env->NewGlobalRef(val.l);
					c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID; // mark as openjdk object
					c->type = metaffi_handle_type;
				}
			}
			
		}break;
		default:
			std::stringstream ss;
			ss << "The metaffi return type " << type.type << " is not handled";
			throw std::runtime_error(ss.str());
	}
}
//--------------------------------------------------------------------
void cdts_java_wrapper::switch_to_object(JNIEnv* env, int i) const
{
	cdt* c = (*this)[i];
	
	switch (c->type)
	{
		case metaffi_int32_type:
		{
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int32) v.i);
		}break;
		case metaffi_int64_type:
		{
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int64)v.j);
		}break;
		case metaffi_int16_type:
		{
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int16)v.s);
		}break;
		case metaffi_int8_type:
		{
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_int8) v.b);
		}break;
		case metaffi_float32_type:
		{
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_float32) v.f);
		}break;
		case metaffi_float64_type:
		{
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, (metaffi_float32) v.d);
		}break;
		case metaffi_bool_type:
		{
			jvalue v = this->to_jvalue(env, i);
			this->set_object(env, i, v.z != JNI_FALSE);
		} break;
	}
}
//--------------------------------------------------------------------
void cdts_java_wrapper::switch_to_primitive(JNIEnv* env, int i, metaffi_type t /*= metaffi_any_type*/) const
{
	char jni_sig = get_jni_primitive_signature_from_object_form_of_primitive(env, i);
	if(jni_sig == 0){
		return;
	}
	
	if(jni_sig == 'L')
	{
		if(is_jstring(env, i))
		{
			if(t != metaffi_any_type && t != metaffi_string8_type && t != metaffi_string16_type && t != metaffi_string32_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received String";
				throw std::runtime_error(ss.str());
			}

			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (std::string)this->get_object_as_string(env, i));
			env->DeleteGlobalRef(obj);
		}
		else
		{
			return;
		}
	}
	
	switch(jni_sig)
	{
		case 'I':
		{
			if(t != metaffi_any_type && t != metaffi_int32_type && t != metaffi_uint32_type && t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Integer";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (metaffi_int32)this->get_object_as_int32(env, i));

			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}

		}
		break;
		
		case 'J':
		{
			if(t != metaffi_any_type && t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Long";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (metaffi_int64)this->get_object_as_int64(env, i));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}break;

		case 'S':
		{
			if(t != metaffi_any_type && t != metaffi_int16_type && t != metaffi_uint16_type && t != metaffi_int32_type && t != metaffi_uint32_type &&
				t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Short";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (metaffi_int16)this->get_object_as_int16(env, i));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
		
		case 'B':
		{
			if(t != metaffi_any_type && t != metaffi_int8_type && t != metaffi_uint8_type && t != metaffi_int16_type && t != metaffi_uint16_type && t != metaffi_int32_type && t != metaffi_uint32_type &&
				t != metaffi_int64_type && t != metaffi_uint64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Byte";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (metaffi_int8)this->get_object_as_int8(env, i));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
		
		case 'C':
		{
			if(t != metaffi_any_type && t != metaffi_uint16_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Char";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (metaffi_uint16)this->get_object_as_uint16(env, i));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
		
		case 'F':
		{
			if(t != metaffi_any_type && t != metaffi_float32_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Float";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (metaffi_float32)this->get_object_as_float32(env, i));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
		
		case 'D':
		{
			if(t != metaffi_any_type && t != metaffi_float64_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Double";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (metaffi_float64)this->get_object_as_float64(env, i));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
		
		case 'Z':
		{
			if(t != metaffi_any_type && t != metaffi_bool_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Boolean";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			this->set(i, (bool)this->get_object_as_bool(env, i));
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
	}
	
}
//--------------------------------------------------------------------
bool cdts_java_wrapper::is_jstring(JNIEnv* env, int index) const
{
	if((*this)[index]->type != metaffi_handle_type){
		return false;
	}
	
	jobject obj = (jobject)((*this)[index]->cdt_val.metaffi_handle_val.val);
	return env->IsInstanceOf(obj, env->FindClass("java/lang/String")) != JNI_FALSE;
}
//--------------------------------------------------------------------
char cdts_java_wrapper::get_jni_primitive_signature_from_object_form_of_primitive(JNIEnv *env, int index) const
{
	if((*this)[index]->type & metaffi_array_type){
		return 'L';
	}
	
	if((*this)[index]->type != metaffi_handle_type){
		return 0;
	}

	if((*this)[index]->type == metaffi_handle_type && (*this)[index]->cdt_val.metaffi_handle_val.runtime_id != OPENJDK_RUNTIME_ID){
		return 0;
	}
	
	jobject obj = (jobject)((*this)[index]->cdt_val.metaffi_handle_val.val);
	
	// Check if the object is an instance of a primitive type
	if (env->IsInstanceOf(obj, env->FindClass("java/lang/Integer"))) {
		return 'I';
	} else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Long"))) {
		return 'J';
	} else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Short"))) {
		return 'S';
	} else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Byte"))) {
		return 'B';
	} else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Character"))) {
		return 'C';
	} else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Float"))) {
		return 'F';
	} else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Double"))) {
		return 'D';
	} else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Boolean"))) {
		return 'Z';
	} else {
		// If it's not a primitive type, it's an object
		return 'L';
	}
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int32 val) const
{
	jclass cls = env->FindClass("java/lang/Integer");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(I)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, bool val) const
{
	jclass cls = env->FindClass("java/lang/Boolean");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(Z)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
//	obj = env->NewGlobalRef(obj);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int8 val) const
{
	jclass cls = env->FindClass("java/lang/Byte");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(B)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_uint16 val) const
{
	jclass cls = env->FindClass("java/lang/Character");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(C)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int16 val) const
{
	jclass cls = env->FindClass("java/lang/Short");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(S)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_int64 val) const
{
	jclass cls = env->FindClass("java/lang/Long");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(J)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_float32 val) const
{
	jclass cls = env->FindClass("java/lang/Float");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(F)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::set_object(JNIEnv* env, int index, metaffi_float64 val) const
{
	jclass cls = env->FindClass("java/lang/Double");
	check_and_throw_jvm_exception(env, true);
	
	jmethodID constructor = env->GetMethodID(cls, "<init>", "(D)V");
	check_and_throw_jvm_exception(env, true);
	
	jobject obj = env->NewObject(cls, constructor, val);
	check_and_throw_jvm_exception(env, true);
	
	cdt* c = (*this)[index];
	c->cdt_val.metaffi_handle_val.val = obj;
	c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;
	c->type = metaffi_handle_type;
}
//--------------------------------------------------------------------
metaffi_int32 cdts_java_wrapper::get_object_as_int32(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Integer"))){
		throw std::runtime_error("expected java/lang/Integer type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "intValue", "()I");
	check_and_throw_jvm_exception(env, true);
	
	jint res = env->CallIntMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
bool cdts_java_wrapper::get_object_as_bool(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Boolean"))){
		throw std::runtime_error("expected java/lang/Boolean type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "booleanValue", "()Z");
	check_and_throw_jvm_exception(env, true);
	
	jboolean res = env->CallBooleanMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res != JNI_FALSE;
}
//--------------------------------------------------------------------
metaffi_int8 cdts_java_wrapper::get_object_as_int8(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Byte"))){
		throw std::runtime_error("expected java/lang/Byte type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "byteValue", "()B");
	check_and_throw_jvm_exception(env, true);
	
	jbyte res = env->CallByteMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
metaffi_uint16 cdts_java_wrapper::get_object_as_uint16(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Character"))){
		throw std::runtime_error("expected java/lang/Character type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "booleanValue", "()V");
	check_and_throw_jvm_exception(env, true);
	
	jchar res = env->CallCharMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
metaffi_int16 cdts_java_wrapper::get_object_as_int16(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Short"))){
		throw std::runtime_error("expected java/lang/Short type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "shortValue", "()S");
	check_and_throw_jvm_exception(env, true);
	
	jshort res = env->CallShortMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
metaffi_int64 cdts_java_wrapper::get_object_as_int64(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Long"))){
		throw std::runtime_error("expected java/lang/Long type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "longValue", "()J");
	check_and_throw_jvm_exception(env, true);
	
	jlong res = env->CallLongMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
metaffi_float32 cdts_java_wrapper::get_object_as_float32(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Float"))){
		throw std::runtime_error("expected java/lang/Float type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "floatValue", "()F");
	check_and_throw_jvm_exception(env, true);
	
	jfloat res = env->CallFloatMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
metaffi_float64 cdts_java_wrapper::get_object_as_float64(JNIEnv* env, int index) const
{
	cdt* c = (*this)[index];
	if(c->type != metaffi_handle_type){
		throw std::runtime_error("expected metaffi_handle type");
	}
	jobject obj = (jobject)c->cdt_val.metaffi_handle_val.val;
	check_and_throw_jvm_exception(env, true);
	
	jclass cls = env->GetObjectClass(obj);
	check_and_throw_jvm_exception(env, true);
	
	if (!env->IsInstanceOf(obj, env->FindClass("java/lang/Double"))){
		throw std::runtime_error("expected java/lang/Double type");
	}
	check_and_throw_jvm_exception(env, true);
	
	jmethodID mid = env->GetMethodID(cls, "doubleValue", "()D");
	check_and_throw_jvm_exception(env, true);
	
	jdouble res = env->CallDoubleMethod(obj, mid);
	check_and_throw_jvm_exception(env, true);
	
	return res;
}
//--------------------------------------------------------------------
std::string cdts_java_wrapper::get_object_as_string(JNIEnv* env, int index) const
{
	if(!is_jstring(env, index)){
		throw std::runtime_error("expected jstring type");
	}
	
	cdt* c = (*this)[index];
	jstring str = (jstring)c->cdt_val.metaffi_handle_val.val;
	
	jclass stringClass = env->GetObjectClass(str);
	check_and_throw_jvm_exception(env, true);
	
	jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
	check_and_throw_jvm_exception(env, true);
	
	auto stringJbytes = (jbyteArray) env->CallObjectMethod(str, getBytes, env->NewStringUTF("UTF-8"));
	check_and_throw_jvm_exception(env, true);
	
	auto length = (size_t)env->GetArrayLength(stringJbytes);
	check_and_throw_jvm_exception(env, true);
	
	jbyte* pBytes = env->GetByteArrayElements(stringJbytes, nullptr);
	check_and_throw_jvm_exception(env, true);
	
	std::string ret = std::string((char*)pBytes, length);
	env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);
	check_and_throw_jvm_exception(env, true);
	
	env->DeleteLocalRef(stringJbytes);
	env->DeleteLocalRef(stringClass);
	
	check_and_throw_jvm_exception(env, true);
	
	return ret;
}
//--------------------------------------------------------------------
bool cdts_java_wrapper::is_jobject(int i) const
{
	cdt* c = (*this)[i];
	return c->type == metaffi_handle_type || c->type == metaffi_string8_type ||
			c->type == metaffi_string16_type || (c->type & metaffi_array_type) != 0;
}
//--------------------------------------------------------------------
