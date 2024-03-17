#include "cdts_java_wrapper.h"
#include <algorithm>
#include <utility>
#include "runtime_id.h"
#include "jni_metaffi_handle.h"
#include "exception_macro.h"
#include "utils/tracer.h"
#include "jni_caller.h"
#include "jbyte_wrapper.h"
#include "jobjectArray_wrapper.h"
#include "jshort_wrapper.h"
#include "jint_wrapper.h"
#include "jlong_wrapper.h"
#include "jfloat_wrapper.h"
#include "jdouble_wrapper.h"
#include "jboolean_wrapper.h"
#include "jchar_wrapper.h"
#include "jstring_wrapper.h"
#include "jobject_wrapper.h"

//--------------------------------------------------------------------
cdts_java_wrapper::cdts_java_wrapper(cdt *cdts, metaffi_size cdts_length):metaffi::runtime::cdts_wrapper(cdts, cdts_length, false)
{
}
//--------------------------------------------------------------------
auto get_on_int8_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "B", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "B", array_length, current_dimensions));
	};
}

template<typename metaffi_type_t>
auto get_on_int8_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_type_t* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = *((std::pair<JNIEnv*, jobject*>*)other_array)->second;
		
		jbyteArray newarr = jbyte_wrapper::new_1d_array(env, length, (const jbyte*)arr);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_int16_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "S", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "S", array_length, current_dimensions));
	};
}

template<typename metaffi_type_t>
auto get_on_int16_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_type_t* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jshortArray newarr = jshort_wrapper::new_1d_array(env, length, (const jshort*)arr);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_int32_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "I", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "I", array_length, current_dimensions));
	};
}

template<typename metaffi_type_t>
auto get_on_int32_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_type_t* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jintArray newarr = jint_wrapper::new_1d_array(env, length, (const jint*)arr);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_int64_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "J", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "J", array_length, current_dimensions));
	};
}

template<typename metaffi_type_t>
auto get_on_int64_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_type_t* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jlongArray newarr = jlong_wrapper::new_1d_array(env, length, (const jlong*)arr);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_float32_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "F", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "F", array_length, current_dimensions));
	};
}

auto get_on_float32_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_float32* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jfloatArray newarr = jfloat_wrapper::new_1d_array(env, length, (const jfloat*)arr);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_float64_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "D", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "D", array_length, current_dimensions));
	};
}

auto get_on_float64_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_float64* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jdoubleArray newarr = jdouble_wrapper::new_1d_array(env, length, (const jdouble*)arr);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_bool_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "Z", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "Z", array_length, current_dimensions));
	};
}

auto get_on_bool_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_bool* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jbooleanArray newarr = jboolean_wrapper::new_1d_array(env, length, (const jboolean*)arr);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_metaffi_handle_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "Ljava/lang/Object;", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "Ljava/lang/Object;", array_length, current_dimensions));
	};
}

auto get_on_metaffi_handle_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, cdt_metaffi_handle* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jobjectArray_wrapper newarr(env, jobjectArray_wrapper::create_array(env, "Ljava/lang/Object;", length, 1));
		
		for(int i=0 ; i<length ; i++)
		{
			cdt_metaffi_handle* curitem = (arr + i);
			if(curitem->runtime_id != OPENJDK_RUNTIME_ID)
			{
				jni_metaffi_handle h(env, curitem->val, curitem->runtime_id);
				newarr.set(i, h.new_jvm_object(env));
			}
			else
			{
				newarr.set(i, static_cast<jobject>(curitem->val));
			}
		}
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = (jobjectArray)newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], (jobjectArray)newarr);
		}
	};
}

auto get_on_char_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "C", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "C", array_length, current_dimensions));
	};
}

template<typename char_t>
auto get_on_char_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, char_t* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jcharArray newarr = jchar_wrapper::new_1d_array(env, (const char_t*)arr, length);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

auto get_on_string_array()
{
	return [](metaffi_size* index, metaffi_size index_size, metaffi_size current_dimensions, metaffi_size array_length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		if(!multidim_array)
		{
			// create the array in the given indices
			jobjectArray arr = jobjectArray_wrapper::create_array(env, "Ljava/lang/String;", array_length, current_dimensions);
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = arr;
			return;
		}
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		
		// traverse by the indices
		for(int i=0 ; i<index_size-1 ; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		arr.set(index[index_size-1], jobjectArray_wrapper::create_array(env, "Ljava/lang/String;", array_length, current_dimensions));
	};
}

template<typename string_t>
auto get_on_string_1d_array()
{
	return [](metaffi_size* index, metaffi_size index_size, string_t* arr, metaffi_size length, void* other_array)
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject&>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject&>*)other_array)->second;
		
		jobjectArray newarr = jstring_wrapper::new_1d_array(env, (const string_t*)arr, length);
		
		if(!multidim_array)
		{
			// create the array in the given indices
			((std::pair<JNIEnv*, jobject&>*)other_array)->second = newarr;
		}
		else
		{
			jobjectArray_wrapper higher_dim_array(env, (jobjectArray)multidim_array);
			
			// traverse by the indices
			for(int i=0 ; i<index_size-1 ; i++)
			{
				higher_dim_array = jobjectArray_wrapper(env, (jobjectArray)higher_dim_array.get(index[i]));
			}
			
			higher_dim_array.set(index[index_size-1], newarr);
		}
	};
}

jvalue cdts_java_wrapper::to_jvalue(JNIEnv* env, int index) const
{
	jvalue jval{};
	cdt* c = (*this)[index];

	switch (c->type)
	{
		case metaffi_int8_type:
			jval.b = c->cdt_val.metaffi_int8_val;
			break;
		case metaffi_int8_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_int8_array, metaffi_int8>(
					c->cdt_val.metaffi_int8_array_val,
					&other_array,
					get_on_int8_array(),
					get_on_int8_1d_array<metaffi_int8>());
		}break;
		case metaffi_uint8_type:
			jval.b = c->cdt_val.metaffi_uint8_val;
			break;
		case metaffi_uint8_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_uint8_array, metaffi_uint8>(
					c->cdt_val.metaffi_uint8_array_val,
					&other_array,
					get_on_int8_array(),
					get_on_int8_1d_array<metaffi_uint8>());
		}break;
		case metaffi_int16_type:
			jval.s = c->cdt_val.metaffi_int16_val;
			break;
		case metaffi_int16_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_int16_array, metaffi_int16>(
					c->cdt_val.metaffi_int16_array_val,
					&other_array,
					get_on_int16_array(),
					get_on_int16_1d_array<metaffi_int16>());
		}break;
		case metaffi_uint16_type:
			jval.s = c->cdt_val.metaffi_uint16_val;
			break;
		case metaffi_uint16_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_uint16_array, metaffi_uint16>(
					c->cdt_val.metaffi_uint16_array_val,
					&other_array,
					get_on_int16_array(),
					get_on_int16_1d_array<metaffi_uint16>());
		}break;
		case metaffi_int32_type:
			jval.i = c->cdt_val.metaffi_int32_val;
			break;
		case metaffi_int32_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_int32_array, metaffi_int32>(
					c->cdt_val.metaffi_int32_array_val,
					&other_array,
					get_on_int32_array(),
					get_on_int32_1d_array<metaffi_int32>());
		}break;
		case metaffi_uint32_type:
			jval.i = c->cdt_val.metaffi_uint32_val;
			break;
		case metaffi_uint32_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_uint32_array, metaffi_uint32>(
					c->cdt_val.metaffi_uint32_array_val,
					&other_array,
					get_on_int32_array(),
					get_on_int32_1d_array<metaffi_uint32>());
		}break;
		case metaffi_int64_type:
			jval.j = c->cdt_val.metaffi_int64_val;
			break;
		case metaffi_int64_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_int64_array, metaffi_int64>(
					c->cdt_val.metaffi_int64_array_val,
					&other_array,
					get_on_int64_array(),
					get_on_int64_1d_array<metaffi_int64>());
		}break;
		case metaffi_uint64_type:
			jval.j = c->cdt_val.metaffi_uint64_val;
			break;
		case metaffi_uint64_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_uint64_array, metaffi_uint64>(
					c->cdt_val.metaffi_uint64_array_val,
					&other_array,
					get_on_int64_array(),
					get_on_int64_1d_array<metaffi_uint64>());
		}break;
		case metaffi_float32_type:
			jval.f = c->cdt_val.metaffi_float32_val;
			break;
		case metaffi_float32_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_float32_array, metaffi_float32>(
					c->cdt_val.metaffi_float32_array_val,
					&other_array,
					get_on_float32_array(),
					get_on_float32_1d_array());
		}break;
		case metaffi_float64_type:
			jval.d = c->cdt_val.metaffi_float64_val;
			break;
		case metaffi_float64_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_float64_array, metaffi_float64>(
					c->cdt_val.metaffi_float64_array_val,
					&other_array,
					get_on_float64_array(),
					get_on_float64_1d_array());
		}break;
		case metaffi_bool_type:
			jval.z = c->cdt_val.metaffi_bool_val ? JNI_TRUE : JNI_FALSE;
			break;
		case metaffi_bool_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_bool_array, metaffi_bool>(
					c->cdt_val.metaffi_bool_array_val,
					&other_array,
					get_on_bool_array(),
					get_on_bool_1d_array());
		}break;
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
		case metaffi_handle_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_handle_array, cdt_metaffi_handle>(
					c->cdt_val.metaffi_handle_array_val,
					&other_array,
					get_on_metaffi_handle_array(),
					get_on_metaffi_handle_1d_array());
		}break;
		case metaffi_char8_type:
		{
			jval.c = (jchar)jchar_wrapper(env, c->cdt_val.metaffi_char8_val);
		}break;
		case metaffi_char8_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_char8_array, metaffi_char8>(
					c->cdt_val.metaffi_char8_array_val,
					&other_array,
					get_on_char_array(),
					get_on_char_1d_array<metaffi_char8>());
		}break;
		case metaffi_string8_type:
		{
			jstring str = env->NewStringUTF((const char*)c->cdt_val.metaffi_string8_val);
			check_and_throw_jvm_exception(env, true);
			jval.l = str;
		}break;
		case metaffi_string8_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_string8_array, metaffi_string8>(
					c->cdt_val.metaffi_string8_array_val,
					&other_array,
					get_on_string_array(),
					get_on_string_1d_array<metaffi_string8>());
		}break;
		case metaffi_char16_type:
			jval.c = (jchar)jchar_wrapper(env, c->cdt_val.metaffi_char16_val);
			break;
		case metaffi_char16_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_char16_array, metaffi_char16>(
					c->cdt_val.metaffi_char16_array_val,
					&other_array,
					get_on_char_array(),
					get_on_char_1d_array<metaffi_char16>());
		}break;
		case metaffi_string16_type:
		{
			jstring_wrapper w(env, c->cdt_val.metaffi_string16_val);
			jval.l = (jstring)w;
		}break;
		case metaffi_string16_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_string16_array, metaffi_string16>(
					c->cdt_val.metaffi_string16_array_val,
					&other_array,
					get_on_string_array(),
					get_on_string_1d_array<metaffi_string16>());
		}break;
		case metaffi_char32_type:
			jval.c = (jchar)jchar_wrapper(env, c->cdt_val.metaffi_char32_val);
			break;
		case metaffi_char32_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_char32_array, metaffi_char32>(
					c->cdt_val.metaffi_char32_array_val,
					&other_array,
					get_on_char_array(),
					get_on_char_1d_array<metaffi_char32>());
		}break;
		case metaffi_string32_type:
		{
			jstring str = (jstring)jstring_wrapper(env, c->cdt_val.metaffi_string32_val);
			jval.l = str;
		}break;
		case metaffi_string32_array_type:
		{
			std::pair<JNIEnv*, jobject&> other_array = {env, jval.l};
			metaffi::runtime::traverse_multidim_array<cdt_metaffi_string32_array, metaffi_string32>(
					c->cdt_val.metaffi_string32_array_val,
					&other_array,
					get_on_string_array(),
					get_on_string_1d_array<metaffi_string32>());
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
auto get_get_array_callback()
{
	return [](metaffi_size* index, metaffi_size index_length, void* other_array)->metaffi_size
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject>*)other_array)->second;
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		for (metaffi_size i = 0; i < index_length; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		return arr.size();
	};
}

template<typename metaffi_type_t, typename jwrapper_t>
auto get_get_1d_array_callback()
{
	return [](metaffi_size* index, metaffi_size index_length, metaffi_size& out_1d_array_length, void* other_array)->metaffi_type_t*
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject>*)other_array)->second;
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		for (metaffi_size i = 0; i < index_length; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		out_1d_array_length = arr.size();
		
		return jwrapper_t::to_c_array<metaffi_type_t>(env, (jarray)((jobject)arr));
	};
}

auto get_get_1d_object_array_callback()
{
	return [](metaffi_size* index, metaffi_size index_length, metaffi_size& out_1d_array_length, void* other_array)->cdt_metaffi_handle*
	{
		JNIEnv* env = ((std::pair<JNIEnv*, jobject>*)other_array)->first;
		jobject multidim_array = ((std::pair<JNIEnv*, jobject>*)other_array)->second;
		
		jobjectArray_wrapper arr(env, (jobjectArray)multidim_array);
		for (metaffi_size i = 0; i < index_length; i++)
		{
			arr = jobjectArray_wrapper(env, (jobjectArray)arr.get(index[i]));
		}
		
		out_1d_array_length = arr.size();
		
		cdt_metaffi_handle* mffi_arr = new cdt_metaffi_handle[out_1d_array_length]{};
		
		for(int i=0 ; i<out_1d_array_length ; i++)
		{
			jobject obj = arr.get(i);
			if(obj)
			{
				mffi_arr[i].runtime_id = OPENJDK_RUNTIME_ID;
				mffi_arr[i].val = obj;
			}
			
			if(jni_metaffi_handle::is_metaffi_handle_wrapper_object(env, obj))
			{
				jni_metaffi_handle h(env, obj);
				mffi_arr[i].val = h.get_handle();
				mffi_arr[i].runtime_id = h.get_runtime_id(); // mark as openjdk object
			}
			else
			{
				// mffi_arr[i].val = env->NewGlobalRef(obj); // Is that required here?!
				mffi_arr[i].val = obj;
				mffi_arr[i].runtime_id = OPENJDK_RUNTIME_ID; // mark as openjdk object
			}
		}
		
		return mffi_arr;
	};
}

//--------------------------------------------------------------------
void cdts_java_wrapper::from_jvalue(JNIEnv* env, jvalue jval, const metaffi_type_info& type, int index) const
{
	cdt* c = (*this)[index];
	c->type = type.type;
	switch (c->type)
	{
		case metaffi_int8_type:
			c->cdt_val.metaffi_int8_val = jbyte_wrapper(jval.b);
			break;
		case metaffi_int8_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_int8_array, metaffi_int8>(
					c->cdt_val.metaffi_int8_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_int8, jbyte_wrapper>());
		} break;
		case metaffi_uint8_type:
			c->cdt_val.metaffi_uint8_val = jbyte_wrapper(jval.b);
			break;
		case metaffi_uint8_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_uint8_array, metaffi_uint8>(
					c->cdt_val.metaffi_uint8_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_uint8, jbyte_wrapper>());
		} break;
		case metaffi_int16_type:
			c->cdt_val.metaffi_int16_val = jval.s;
			break;
		case metaffi_int16_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_int16_array, metaffi_int16>(
					c->cdt_val.metaffi_int16_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_int16, jshort_wrapper>());
		} break;
		case metaffi_uint16_type:
			c->cdt_val.metaffi_uint16_val = jval.s;
			break;
		case metaffi_uint16_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_uint16_array, metaffi_uint16>(
					c->cdt_val.metaffi_uint16_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_uint16, jshort_wrapper>());
		} break;
		case metaffi_int32_type:
			c->cdt_val.metaffi_int32_val = jval.i;
			break;
		case metaffi_int32_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_int32_array, metaffi_int32>(
					c->cdt_val.metaffi_int32_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_int32, jint_wrapper>());
		} break;
		case metaffi_uint32_type:
			c->cdt_val.metaffi_uint32_val = jval.i;
			break;
		case metaffi_uint32_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_uint32_array, metaffi_uint32>(
					c->cdt_val.metaffi_uint32_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_uint32, jint_wrapper>());
		} break;
		case metaffi_int64_type:
			c->cdt_val.metaffi_int64_val = jval.j;
			break;
		case metaffi_int64_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_int64_array, metaffi_int64>(
					c->cdt_val.metaffi_int64_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_int64, jlong_wrapper>());
		} break;
		case metaffi_uint64_type:
			c->cdt_val.metaffi_uint64_val = jval.j;
			break;
		case metaffi_uint64_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_uint64_array, metaffi_uint64>(
					c->cdt_val.metaffi_uint64_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_uint64, jlong_wrapper>());
		} break;
		case metaffi_float32_type:
			c->cdt_val.metaffi_float32_val = jval.f;
			break;
		case metaffi_float32_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_float32_array, metaffi_float32>(
					c->cdt_val.metaffi_float32_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_float32, jfloat_wrapper>());
		}break;
		case metaffi_float64_type:
			c->cdt_val.metaffi_float64_val = jval.d;
			break;
		case metaffi_float64_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_float64_array, metaffi_float64>(
					c->cdt_val.metaffi_float64_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_float64, jdouble_wrapper>());
		}break;
		case metaffi_handle_type:
			// if val.l is MetaFFIHandle - get its inner data
			if(jni_metaffi_handle::is_metaffi_handle_wrapper_object(env, jval.l))
			{
				jni_metaffi_handle h(env, jval.l);
				c->cdt_val.metaffi_handle_val.val = h.get_handle();
				c->cdt_val.metaffi_handle_val.runtime_id = h.get_runtime_id(); // mark as openjdk object
				c->type = metaffi_handle_type;
			}
			else
			{
				c->cdt_val.metaffi_handle_val.val = env->NewGlobalRef(jval.l);
				c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID; // mark as openjdk object
				c->type = metaffi_handle_type;
			}
			break;
		case metaffi_handle_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_handle_array, cdt_metaffi_handle>(
					c->cdt_val.metaffi_handle_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_object_array_callback());
		}break;
		case metaffi_callable_type:
			// get from val.l should be Caller class
			if(!is_caller_class(env, jval.l))
			{
				throw std::runtime_error("caller_type expects metaffi.Caller object");
			}
			fill_callable_cdt(env, jval.l, c->cdt_val.metaffi_callable_val.val,
								c->cdt_val.metaffi_callable_val.parameters_types, c->cdt_val.metaffi_callable_val.params_types_length,
								c->cdt_val.metaffi_callable_val.retval_types, c->cdt_val.metaffi_callable_val.retval_types_length);
			break;
		case metaffi_bool_type:
			c->cdt_val.metaffi_bool_val = jval.z;
			break;
		case metaffi_bool_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_bool_array, metaffi_bool>(
					c->cdt_val.metaffi_bool_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_bool, jboolean_wrapper>());
		}break;
		case metaffi_char8_type:
			c->cdt_val.metaffi_char8_val = ((std::u8string)jchar_wrapper(env, jval.c))[0];
			break;
		case metaffi_char8_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_char8_array, metaffi_char8>(
					c->cdt_val.metaffi_char8_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_char8, jchar_wrapper>());
		}
		case metaffi_string8_type:
		{
			jstring_wrapper w(env, (jstring)jval.l);
			c->cdt_val.metaffi_string8_val = (metaffi_string8)((const char8_t*)w);
		} break;
		case metaffi_string8_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_string8_array, metaffi_string8>(
					c->cdt_val.metaffi_string8_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_string8, jstring_wrapper>());
		} break;
		case metaffi_char16_type:
			c->cdt_val.metaffi_char16_val = (char16_t)jchar_wrapper(env, jval.c);
			break;
		case metaffi_char16_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_char16_array, metaffi_char16>(
					c->cdt_val.metaffi_char16_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_char16, jchar_wrapper>());
		}
		case metaffi_string16_type:
		{
			jstring_wrapper w(env, (jstring)jval.l);
			c->cdt_val.metaffi_string16_val = (metaffi_string16)((const char16_t*)w);
		} break;
		case metaffi_string16_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_string16_array, metaffi_string16>(
					c->cdt_val.metaffi_string16_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_string16, jstring_wrapper>());
		} break;
		case metaffi_char32_type:
			c->cdt_val.metaffi_char32_val = (char32_t)jchar_wrapper(env, jval.c);
			break;
		case metaffi_char32_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_char32_array, metaffi_char32>(
					c->cdt_val.metaffi_char32_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_char32, jchar_wrapper>());
		}
		case metaffi_string32_type:
		{
			jstring_wrapper w(env, (jstring)jval.l);
			c->cdt_val.metaffi_string32_val = (metaffi_string32)((const char32_t*)w);
		} break;
		case metaffi_string32_array_type:
		{
			auto other_array = std::make_pair(env, jval.l);
			metaffi::runtime::construct_multidim_array<cdt_metaffi_string32_array, metaffi_string32>(
					c->cdt_val.metaffi_string32_array_val,
					type.dimensions,
					&other_array,
					get_get_array_callback(),
					get_get_1d_array_callback<metaffi_string32, jstring_wrapper>());
		} break;
		case metaffi_any_type:
		{
			// returned type is an object, we need to *try* and switch it to primitive if it
			// can be converted to primitive
			c->cdt_val.metaffi_handle_val.val = jval.l;
			c->cdt_val.metaffi_handle_val.runtime_id = OPENJDK_RUNTIME_ID;

			switch_to_primitive(env, index);
			
			if(c->type == metaffi_handle_type)
			{
				if(jni_metaffi_handle::is_metaffi_handle_wrapper_object(env, jval.l))
				{
					jni_metaffi_handle h(env, jval.l);
					c->cdt_val.metaffi_handle_val.val = h.get_handle();
					c->cdt_val.metaffi_handle_val.runtime_id = h.get_runtime_id(); // mark as openjdk object
					c->type = metaffi_handle_type;
				}
				else
				{
					c->cdt_val.metaffi_handle_val.val = env->NewGlobalRef(jval.l);
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

			if(t == metaffi_any_type || t == metaffi_string8_type)
			{
				jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
				this->set(i, jobject_wrapper(env, obj).get_as_string8());
				env->DeleteGlobalRef(obj);
			}
			else if(t == metaffi_string16_type)
			{
				jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
				this->set(i, jobject_wrapper(env, obj).get_as_string16());
				env->DeleteGlobalRef(obj);
			}
			else if(t == metaffi_string32_type)
			{
				jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
				this->set(i, jobject_wrapper(env, obj).get_as_string32());
				env->DeleteGlobalRef(obj);
			}
			else
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received String";
				throw std::runtime_error(ss.str());
			}
			
		}
		else
		{
			(*this)[i]->type = metaffi_handle_type;
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
			this->set(i, (metaffi_int32)jobject_wrapper(env, obj).get_as_int32());

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
			this->set(i, (metaffi_int64)jobject_wrapper(env, obj).get_as_int64());
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
			this->set(i, (metaffi_int16)jobject_wrapper(env, obj).get_as_int16());
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
			this->set(i, (metaffi_int8)jobject_wrapper(env, obj).get_as_int8());
			if(env->GetObjectRefType(obj) == JNIGlobalRefType)
			{
				env->DeleteGlobalRef(obj);
			}
		}
		break;
		
		case 'C':
		{
			if(t != metaffi_any_type && t != metaffi_char8_type && t != metaffi_char16_type && t != metaffi_char32_type)
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Char";
				throw std::runtime_error(ss.str());
			}
			jobject obj = (jobject)(*this)[i]->cdt_val.metaffi_handle_val.val;
			if(t == metaffi_any_type || t == metaffi_char8_type)
			{
				this->set(i, (char8_t)jobject_wrapper(env, obj).get_as_char8());
			}
			else if(t == metaffi_char16_type)
			{
				this->set(i, (char16_t)jobject_wrapper(env, obj).get_as_char16());
			}
			else if(t == metaffi_char32_type)
			{
				this->set(i, (char32_t)jobject_wrapper(env, obj).get_as_char32());
			}
			else
			{
				std::stringstream ss;
				ss << "Expected metaffi type: " << t << " while received Char";
				throw std::runtime_error(ss.str());
			}
			
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
			this->set(i, (metaffi_float32)jobject_wrapper(env, obj).get_as_float32());
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
			this->set(i, (metaffi_float64)jobject_wrapper(env, obj).get_as_float64());
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
			this->set(i, (bool)jobject_wrapper(env, obj).get_as_bool());
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
	
	if((*this)[index]->type != metaffi_handle_type && (*this)[index]->type != metaffi_any_type){
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
bool cdts_java_wrapper::is_jobject(int i) const
{
	cdt* c = (*this)[i];
	return c->type == metaffi_handle_type || c->type == metaffi_string8_type ||
			c->type == metaffi_string16_type || (c->type & metaffi_array_type) != 0;
}
//--------------------------------------------------------------------
