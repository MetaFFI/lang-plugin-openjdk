#include "cdts_java_wrapper.h"
#include <algorithm>
#include <utility>
#include "runtime_id.h"
#include "jni_metaffi_handle.h"
#include "exception_macro.h"
#include "utils/tracer.h"

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
		case metaffi_int32_array_type:
		{
			jintArray arr = env->NewIntArray(c->cdt_val.metaffi_int32_array_val.dimensions_lengths[0]);
			check_and_throw_jvm_exception(env, true);
			env->SetIntArrayRegion(arr, 0, c->cdt_val.metaffi_int32_array_val.dimensions_lengths[0], reinterpret_cast<jint*>(c->cdt_val.metaffi_int32_array_val.vals));
			check_and_throw_jvm_exception(env, true);
			jval.l = arr;
		} break;
		case metaffi_int64_array_type:
		{
			jlongArray arr = env->NewLongArray(c->cdt_val.metaffi_int64_array_val.dimensions_lengths[0]);
			check_and_throw_jvm_exception(env, true);
			env->SetLongArrayRegion(arr, 0, c->cdt_val.metaffi_int64_array_val.dimensions_lengths[0], reinterpret_cast<jlong*>(c->cdt_val.metaffi_int64_array_val.vals));
			check_and_throw_jvm_exception(env, true);
			jval.l = arr;
		} break;
		case metaffi_int16_array_type:
		{
			jshortArray arr = env->NewShortArray(c->cdt_val.metaffi_int16_array_val.dimensions_lengths[0]);
			check_and_throw_jvm_exception(env, true);
			env->SetShortArrayRegion(arr, 0, c->cdt_val.metaffi_int16_array_val.dimensions_lengths[0], reinterpret_cast<jshort*>(c->cdt_val.metaffi_int16_array_val.vals));
			check_and_throw_jvm_exception(env, true);
			jval.l = arr;
		} break;
		case metaffi_int8_array_type:
		{
			jbyteArray arr = env->NewByteArray(c->cdt_val.metaffi_int8_array_val.dimensions_lengths[0]);
			check_and_throw_jvm_exception(env, true);
			env->SetByteArrayRegion(arr, 0, c->cdt_val.metaffi_int8_array_val.dimensions_lengths[0], reinterpret_cast<jbyte*>(c->cdt_val.metaffi_int8_array_val.vals));
			check_and_throw_jvm_exception(env, true);
			jval.l = arr;
		} break;
		case metaffi_float32_array_type:
		{
			jfloatArray arr = env->NewFloatArray(c->cdt_val.metaffi_float32_array_val.dimensions_lengths[0]);
			check_and_throw_jvm_exception(env, true);
			env->SetFloatArrayRegion(arr, 0, c->cdt_val.metaffi_float32_array_val.dimensions_lengths[0], reinterpret_cast<jfloat*>(c->cdt_val.metaffi_float32_array_val.vals));
			check_and_throw_jvm_exception(env, true);
			jval.l = arr;
		} break;
		case metaffi_float64_array_type:
		{
			jdoubleArray arr = env->NewDoubleArray(c->cdt_val.metaffi_float64_array_val.dimensions_lengths[0]);
			check_and_throw_jvm_exception(env, true);
			env->SetDoubleArrayRegion(arr, 0, c->cdt_val.metaffi_float64_array_val.dimensions_lengths[0], reinterpret_cast<jdouble*>(c->cdt_val.metaffi_float64_array_val.vals));
			check_and_throw_jvm_exception(env, true);
			jval.l = arr;
		} break;
		case metaffi_handle_array_type:
		{
			jobjectArray arr = env->NewObjectArray(c->cdt_val.metaffi_handle_array_val.dimensions_lengths[0], env->FindClass("java/lang/Object"), nullptr);
			check_and_throw_jvm_exception(env, true);
			for (int i = 0; i < c->cdt_val.metaffi_handle_array_val.dimensions_lengths[0]; ++i)
			{
				env->SetObjectArrayElement(arr, i, reinterpret_cast<jobject>(c->cdt_val.metaffi_handle_array_val.vals[i].val));
				check_and_throw_jvm_exception(env, true);
			}
			jval.l = arr;
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
		default:
			std::stringstream ss;
			ss << "Unsupported type to convert to Java: " << c->type;
			throw std::runtime_error(ss.str());
	}
	
	return jval;
}
//--------------------------------------------------------------------
void cdts_java_wrapper::from_jvalue(JNIEnv* env, jvalue val, metaffi_type type, int index) const
{
	cdt* c = (*this)[index];
	c->type = type;
	switch (type)
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
			jlongArray arr = static_cast<jlongArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			c->cdt_val.metaffi_int64_array_val.dimensions = 1;
			c->cdt_val.metaffi_int64_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_int64_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_int64_array_val.vals = new jlong[len];
			env->SetLongArrayRegion(arr, 0, len, c->cdt_val.metaffi_int64_array_val.vals);
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_int16_array_type:
		{
			c->type = metaffi_int16_array_type;
			jshortArray arr = static_cast<jshortArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			c->cdt_val.metaffi_int16_array_val.dimensions = 1;
			c->cdt_val.metaffi_int16_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_int16_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_int16_array_val.vals = new jshort[len];
			env->SetShortArrayRegion(arr, 0, len, c->cdt_val.metaffi_int16_array_val.vals);
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_int8_array_type:
		{
			c->type = metaffi_int8_array_type;
			jbyteArray arr = static_cast<jbyteArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			c->cdt_val.metaffi_int8_array_val.dimensions = 1;
			c->cdt_val.metaffi_int8_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_int8_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_int8_array_val.vals = new jbyte[len];
			env->SetByteArrayRegion(arr, 0, len, c->cdt_val.metaffi_int8_array_val.vals);
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_bool_array_type:
		{
			c->type = metaffi_bool_array_type;
			jbooleanArray arr = static_cast<jbooleanArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			c->cdt_val.metaffi_bool_array_val.dimensions = 1;
			c->cdt_val.metaffi_bool_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_bool_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_bool_array_val.vals = new jboolean[len];
			env->SetBooleanArrayRegion(arr, 0, len, c->cdt_val.metaffi_bool_array_val.vals);
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_float32_array_type:
		{
			c->type = metaffi_float32_array_type;
			jfloatArray arr = static_cast<jfloatArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			c->cdt_val.metaffi_float32_array_val.dimensions = 1;
			c->cdt_val.metaffi_float32_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_float32_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_float32_array_val.vals = new jfloat[len];
			env->SetFloatArrayRegion(arr, 0, len, c->cdt_val.metaffi_float32_array_val.vals);
			check_and_throw_jvm_exception(env, true);
		} break;
		case metaffi_float64_array_type:
		{
			c->type = metaffi_float64_array_type;
			jdoubleArray arr = static_cast<jdoubleArray>(val.l);
			int len = env->GetArrayLength(arr);
			check_and_throw_jvm_exception(env, true);
			c->cdt_val.metaffi_float64_array_val.dimensions = 1;
			c->cdt_val.metaffi_float64_array_val.dimensions_lengths = new metaffi_size[1];
			c->cdt_val.metaffi_float64_array_val.dimensions_lengths[0] = len;
			c->cdt_val.metaffi_float64_array_val.vals = new jdouble[len];
			env->SetDoubleArrayRegion(arr, 0, len, c->cdt_val.metaffi_float64_array_val.vals);
			check_and_throw_jvm_exception(env, true);
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
			c->cdt_val.metaffi_handle_val.runtime_id = 0;
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
			ss << "The metaffi return type " << type << " is not handled";
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
			env->DeleteGlobalRef(obj);
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
			env->DeleteGlobalRef(obj);
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
			env->DeleteGlobalRef(obj);
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
			env->DeleteGlobalRef(obj);
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
			env->DeleteGlobalRef(obj);
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
			env->DeleteGlobalRef(obj);
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
			env->DeleteGlobalRef(obj);
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
			env->DeleteGlobalRef(obj);
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
