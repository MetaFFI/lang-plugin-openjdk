#include "cdts_java.h"
#include "runtime/cdt_capi_loader.h"
#include <mutex>
#include "objects_table.h"
#include <utility>
#include "utils/scope_guard.hpp"
#include <typeinfo>

using namespace metaffi::runtime;
std::once_flag load_capi_flag;

struct java_cdts_build_callbacks : public cdts_build_callbacks_interface
{
	JNIEnv* env = nullptr;
	
	explicit java_cdts_build_callbacks(JNIEnv* env):env(env){}
	
	//--------------------------------------------------------------------
	template<typename T, typename jT>
	void set_numeric_to_jobject(jobjectArray objects, int index, const T& val, jclass cls, jmethodID meth, const std::function<jT(T)>& ConvFunc)
	{
		jT jval = ConvFunc(val);
		env->SetObjectArrayElement(objects, index, env->NewObject(cls, meth, jval));
	}
	//--------------------------------------------------------------------
	template<typename T, typename char_t>
	void set_string_to_jobject(jobjectArray objects, int index, const T& val, const metaffi_size& size, const std::function<jstring(const char_t*, metaffi_size)>& ConvFunc)
	{
		jstring jstr = ConvFunc((char_t*)val, size);
		env->SetObjectArrayElement(objects, index, jstr);
	}
	//--------------------------------------------------------------------
	template<typename T, typename jT, typename jTArray>
	void set_numeric_array_to_jobject(jobjectArray objects, int cdts_index, const metaffi::runtime::numeric_n_array_wrapper<T>& arr_wrap,
	                                  const std::function<jTArray(jsize)>& new_array_t,
	                                  const std::function<void(jTArray, jsize, const jT*)>& set_array_t)
	{
		jTArray obj_array = new_array_t((jsize)arr_wrap.dimensions_lengths[0]);
		metaffi_size index[1] = { 0 };
		set_array_t(obj_array, (jsize)arr_wrap.dimensions_lengths[0], (jT*)arr_wrap.get_array_at(index, 1));
		env->SetObjectArrayElement(objects, (jsize)index[0], obj_array);
	}
	//--------------------------------------------------------------------
	template<typename T, typename char_t>
	void set_string_array_to_jobject(jobjectArray values_to_set, int cdts_index, const metaffi::runtime::string_n_array_wrapper<T>& arr_wrap, const std::function<jstring(const char_t*, jsize)>& c_to_jstring)
	{
		metaffi_size size = arr_wrap.dimensions_lengths[0];
		if(size == 0){
			env->SetObjectArrayElement((jobjectArray)values_to_set, cdts_index, nullptr);
			return;
		}
		
		metaffi_size arrindex[1] = { 0 };
		jobjectArray res = env->NewObjectArray((jsize)size, cdts_java::string_class, nullptr);
		
		for(int i=0 ; i<size ; i++)
		{
			arrindex[0] = i;
			T curstr;
			metaffi_size curlen;
			arr_wrap.get_elem_at(arrindex, 1, &curstr, &curlen);
			jstring cur_jstr = c_to_jstring(curstr, curlen);
			env->SetObjectArrayElement(res, i, cur_jstr);
		}
		
		env->SetObjectArrayElement((jobjectArray)values_to_set, cdts_index, res);
	}
	
	//====================================================================
	
	template<typename T>
	void set_numeric_to_cdts(jobjectArray objectArray, int index, T& val, const std::function<T(jobject)>& jobject_to_c)
	{
		jobject val_to_set = env->GetObjectArrayElement(objectArray, index);
		val = jobject_to_c(val_to_set);
	}
	
	template<typename T, typename jarray_t>
	void set_numeric_array_to_cdts(jobjectArray objectArray, int index, T*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, const std::function<T*(jarray_t)>& jobject_to_c)
	{
		jarray_t val_to_set = (jarray_t)env->GetObjectArrayElement(objectArray, index);
		jsize len = env->GetArrayLength(val_to_set);
		if(len == 0)
		{
			arr = nullptr;
			dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
			dimensions_lengths[0] = 0;
			return;
		}
		
		dimensions = 1; // TODO: handle multi-dimensions
		dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		dimensions_lengths[0] = len;
		
		arr = jobject_to_c(val_to_set);
	}
	
	template<typename T, typename char_t>
	void set_string_to_cdts(jobject obj, int index, T& val, metaffi_size& s, const std::function<char_t*(jstring)>& jstring_to_c, const std::function<metaffi_size(jstring)>& jstring_size)
	{
		jstring curobj = (jstring)env->GetObjectArrayElement((jobjectArray)obj, index);
		
		val = jstring_to_c(curobj);
		s = jstring_size(curobj);
	}
	
	template<typename T, typename char_t>
	void set_string_array_to_cdts(jobject obj, int index, T*& arr, metaffi_size*& strings_lengths, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, const std::function<char_t*(jstring)>& jstring_to_c, const std::function<metaffi_size(jstring)>& jstring_size)
	{
		jobjectArray curarray = (jobjectArray)env->GetObjectArrayElement((jobjectArray)obj, index);
		
		jsize size = 0;
		if(!curarray || (size = env->GetArrayLength(curarray), size == 0))
		{
			arr = nullptr;
			dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
			dimensions_lengths[0] = 0;
		}
		
		dimensions = 1; // TODO: handle multi-dimensions
		dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		dimensions_lengths[0] = size;
		
		strings_lengths = (metaffi_size*)malloc(sizeof(metaffi_size)*size);
		arr = (T*)malloc(sizeof(T)*size);
		
		for(int i=0 ; i<size ; i++)
		{
			jstring cur_item = (jstring)env->GetObjectArrayElement(curarray, i);
			
			arr[i] = jstring_to_c(cur_item);
			strings_lengths[i] = jstring_size(cur_item);
			
		}
	}
	
	void set_metaffi_float32_array(void* values_to_set, int index, metaffi_float32*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_float32, jfloatArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                        [this](jfloatArray pjarr)->metaffi_float32*{ jboolean is_copy = 0; return env->GetFloatArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_float32(void* values_to_set, int index, metaffi_float32& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_float32>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                     [this](jobject val_to_set)->metaffi_float32 {return env->CallFloatMethod(val_to_set, cdts_java::float_get_value);});
		
	}
	
	void set_metaffi_float64(void* values_to_set, int index, metaffi_float64& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_float64>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                     [this](jobject val_to_set)->metaffi_float64 {return env->CallDoubleMethod(val_to_set, cdts_java::double_get_value);});
		
	}
	
	void set_metaffi_float64_array(void* values_to_set, int index, metaffi_float64*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_float64, jdoubleArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                         [this](jdoubleArray pjarr)->metaffi_float64*{ jboolean is_copy = 0; return env->GetDoubleArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_int8(void* values_to_set, int index, metaffi_int8& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_int8>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                  [this](jobject val_to_set)->metaffi_int8 {return env->CallByteMethod(val_to_set, cdts_java::byte_get_value);});
		
	}
	
	void set_metaffi_int8_array(void* values_to_set, int index, metaffi_int8*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_int8, jbyteArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                    [this](jbyteArray pjarr)->metaffi_int8*{ jboolean is_copy = 0; return env->GetByteArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_int16(void* values_to_set, int index, metaffi_int16& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_int16>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                   [this](jobject val_to_set)->metaffi_int16 {return env->CallShortMethod(val_to_set, cdts_java::short_get_value);});
		
	}
	
	void set_metaffi_int16_array(void* values_to_set, int index, metaffi_int16*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_int16, jshortArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                      [this](jshortArray pjarr)->metaffi_int16*{ jboolean is_copy = 0; return env->GetShortArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_int32(void* values_to_set, int index, metaffi_int32& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_int32>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                   [this](jobject val_to_set)->metaffi_int32 {return env->CallIntMethod(val_to_set, cdts_java::int_get_value);});
		
	}
	
	void set_metaffi_int32_array(void* values_to_set, int index, metaffi_int32*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_int32, jintArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                    [this](jintArray pjarr)->metaffi_int32*{ jboolean is_copy = 0; return (metaffi_int32*)env->GetIntArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_int64(void* values_to_set, int index, metaffi_int64& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_int64>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                   [this](jobject val_to_set)->metaffi_int64 {return env->CallLongMethod(val_to_set, cdts_java::long_get_value);});
		
	}
	
	void set_metaffi_int64_array(void* values_to_set, int index, metaffi_int64*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_int64, jlongArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                     [this](jlongArray pjarr)->metaffi_int64*{ jboolean is_copy = 0; return env->GetLongArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_uint8_array(void* values_to_set, int index, metaffi_uint8*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_uint8, jbyteArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
															[this](jbyteArray pjarr)->metaffi_uint8*{ jboolean is_copy = 0; return (metaffi_uint8*)env->GetByteArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_uint8(void* values_to_set, int index, metaffi_uint8& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_uint8>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                   [this](jobject val_to_set)->metaffi_uint8 {return env->CallByteMethod(val_to_set, cdts_java::byte_get_value);});
		
	}
	
	void set_metaffi_uint16(void* values_to_set, int index, metaffi_uint16& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_uint16>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                    [this](jobject val_to_set)->metaffi_uint16 {return env->CallShortMethod(val_to_set, cdts_java::short_get_value);});
		
	}
	
	void set_metaffi_uint16_array(void* values_to_set, int index, metaffi_uint16*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_uint16, jshortArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                       [this](jshortArray pjarr)->metaffi_uint16*{ jboolean is_copy = 0; return (metaffi_uint16*)env->GetShortArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_uint32(void* values_to_set, int index, metaffi_uint32& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_uint32>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                    [this](jobject val_to_set)->metaffi_uint32 {return env->CallIntMethod(val_to_set, cdts_java::int_get_value);});
		
	}
	
	void set_metaffi_uint32_array(void* values_to_set, int index, metaffi_uint32*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_uint32, jintArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                     [this](jintArray pjarr)->metaffi_uint32*{ jboolean is_copy = 0; return (metaffi_uint32*)env->GetIntArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_uint64(void* values_to_set, int index, metaffi_uint64& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_uint64>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                    [this](jobject val_to_set)->metaffi_uint64 {return env->CallLongMethod(val_to_set, cdts_java::long_get_value);});
	}
	
	void set_metaffi_uint64_array(void* values_to_set, int index, metaffi_uint64*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_uint64, jlongArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                      [this](jlongArray pjarr)->metaffi_uint64*{ jboolean is_copy = 0; return (metaffi_uint64*)env->GetLongArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_bool(void* values_to_set, int index, metaffi_bool& val_to_set, int starting_index) override
	{
		set_numeric_to_cdts<metaffi_bool>((jobjectArray)values_to_set, index+starting_index, val_to_set,
		                                  [this](jobject val_to_set)->metaffi_bool {return env->CallBooleanMethod(val_to_set, cdts_java::boolean_get_value);});
		
	}
	
	void set_metaffi_bool_array(void* values_to_set, int index, metaffi_bool*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_numeric_array_to_cdts<metaffi_bool, jbooleanArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions,
		                                                       [this](jbooleanArray pjarr)->metaffi_bool*{ jboolean is_copy = 0; return (metaffi_bool*)env->GetBooleanArrayElements(pjarr, &is_copy); });
		
	}
	
	void set_metaffi_handle_array(void* values_to_set, int index, metaffi_handle*& parray_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		auto set_object = [this, &parray_dimensions_lengths](jobject jobj)->metaffi_handle*
		{
			if(!jobj || env->GetArrayLength((jarray)jobj) == 0){
				return nullptr;
			}
			
			// check if first item is a java object
			jobject elem = env->GetObjectArrayElement((jobjectArray)jobj, 0);
			metaffi_handle* res = (metaffi_handle*)malloc(sizeof(metaffi_handle)*parray_dimensions_lengths[0]);
			if(env->IsInstanceOf(jobj, cdts_java::metaffi_handle_class))
			{
				// metaffi handle, pass as it is.
				for(int i=0 ; i<parray_dimensions_lengths[0] ; i++)
				{
					jobject curObject = env->GetObjectArrayElement((jobjectArray)jobj, i);
					res[i] = (metaffi_handle)env->CallLongMethod(curObject, cdts_java::metaffi_handle_get_value);
				}
			}
			else
			{
				// java objects
				for(int i=0 ; i<parray_dimensions_lengths[0] ; i++)
				{
					jobject curObject = env->GetObjectArrayElement((jobjectArray)jobj, i);
					if(!openjdk_objects_table::instance().contains(curObject))
					{
						jobject g_jobj = env->NewGlobalRef(curObject);
						openjdk_objects_table::instance().set(g_jobj);
						res[i] = g_jobj;
					}
					else
					{
						res[i] = curObject;
					}
				}
			}
			
			return res;
		};
		
		set_numeric_array_to_cdts<metaffi_handle, jobjectArray>((jobjectArray)values_to_set, index+starting_index, parray_to_set, parray_dimensions_lengths, array_dimensions, set_object);
		
	}
	
	void set_metaffi_handle(void* values_to_set, int index, metaffi_handle& val_to_set, int starting_index) override
	{
		auto set_object = [this](jobject jobj)->metaffi_handle
		{
			if(!jobj){
				return nullptr;
			}
			
			if(!openjdk_objects_table::instance().contains(jobj))
			{
				// if metaffi handle - pass as it is.
				if(env->IsInstanceOf(jobj, cdts_java::metaffi_handle_class))
				{
					return (metaffi_handle)env->CallLongMethod(jobj, cdts_java::metaffi_handle_get_value);
				}
				
				// a java object
				jobject g_jobj = env->NewGlobalRef(jobj);
				openjdk_objects_table::instance().set(g_jobj);
				return g_jobj;
			}
			else
			{
				return (metaffi_handle)jobj;
			}
			
		};
		
		set_numeric_to_cdts<metaffi_handle>((jobjectArray)values_to_set, index+starting_index, val_to_set, set_object);
		
	}
	
	void set_metaffi_string8_array(void* values_to_set, int index, metaffi_string8*& parray_to_set, metaffi_size*& pelements_lengths_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_string_array_to_cdts<metaffi_string8, char>((jobject)values_to_set, index+starting_index, parray_to_set, pelements_lengths_to_set, parray_dimensions_lengths, array_dimensions,
		                                                [this](jstring jstr)->metaffi_string8{ jboolean is_copy = JNI_TRUE; return (metaffi_string8)env->GetStringUTFChars(jstr, &is_copy); },
		                                                [this](jstring jstr)->metaffi_size{ return env->GetStringUTFLength(jstr); });
		
	}
	
	void set_metaffi_string8(void* values_to_set, int index, metaffi_string8& val_to_set, metaffi_size& val_length, int starting_index) override
	{
		set_string_to_cdts<metaffi_string8, char>((jobject)values_to_set, index+starting_index, val_to_set, val_length,
		                                          [this](jstring jstr)->metaffi_string8{
														if(!jstr){ return nullptr; }
														jboolean is_copy = JNI_TRUE;
														return (metaffi_string8)env->GetStringUTFChars(jstr, &is_copy);
													},
		                                          [this](jstring jstr)->metaffi_size{
			                                            if(!jstr){ return 0; }
														return env->GetStringUTFLength(jstr);
													});
	}
	
	void set_metaffi_string16_array(void* values_to_set, int index, metaffi_string16*& parray_to_set, metaffi_size*& pelements_lengths_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		set_string_array_to_cdts<metaffi_string16, char16_t>((jobject)values_to_set, index+starting_index, parray_to_set, pelements_lengths_to_set, parray_dimensions_lengths, array_dimensions,
		                                                     [this](jstring jstr)->metaffi_string16{ jboolean is_copy = JNI_TRUE; return (metaffi_string16)env->GetStringChars(jstr, &is_copy); },
		                                                     [this](jstring jstr)->metaffi_size{ return env->GetStringUTFLength(jstr); });
		
	}
	
	void set_metaffi_string16(void* values_to_set, int index, metaffi_string16& val_to_set, metaffi_size& val_length, int starting_index) override
	{
		set_string_to_cdts<metaffi_string16, char16_t>((jobject)values_to_set, index+starting_index, val_to_set, val_length,
		                                               [this](jstring jstr)->metaffi_string16{ jboolean is_copy = JNI_TRUE; return (metaffi_string16)env->GetStringChars(jstr, &is_copy); },
		                                               [this](jstring jstr)->metaffi_size{ return env->GetStringUTFLength(jstr); });
				
	}
	
	void set_metaffi_string32(void* values_to_set, int index, metaffi_string32& val_to_set, metaffi_size& val_length, int starting_index) override
	{
		throw std::runtime_error("Does not support UTF32");
		
	}
	
	void set_metaffi_string32_array(void* values_to_set, int index, metaffi_string32*& parray_to_set, metaffi_size*& pelements_lengths_to_set, metaffi_size*& parray_dimensions_lengths, metaffi_size& array_dimensions, metaffi_bool& is_free_required, int starting_index) override
	{
		throw std::runtime_error("Does not support UTF32");
	}
	
	metaffi_type resolve_dynamic_type(int index, void* values_to_set) override
	{
		jobject curObj = env->GetObjectArrayElement((jobjectArray)values_to_set, index);
		
		jclass objClass = env->GetObjectClass(curObj);
		jclass cls = env->FindClass("java/lang/Class");
		jmethodID mid_getName = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");
		jstring name = (jstring)env->CallObjectMethod(objClass, mid_getName);
		
		jboolean is_copy;
		const char* objcls = env->GetStringUTFChars((jstring)name, &is_copy);
		

		#define if_class(cls, metaffitype) \
			if(strcmp(cls, objcls) == 0){ \
				return metaffitype; \
			}
		
		if_class("java.lang.Float", metaffi_float32_type)
		else if_class("[java.lang.Float", metaffi_float32_array_type)
		
		else if_class("java.lang.Double", metaffi_float64_type)
		else if_class("[java.lang.Double", metaffi_float64_array_type)
		
		else if_class("java.lang.Byte", metaffi_int8_type)
		else if_class("[java.lang.Byte", metaffi_int8_array_type)
		
		else if_class("java.lang.Short", metaffi_int16_type)
		else if_class("[java.lang.Short", metaffi_int16_array_type)
		
		else if_class("java.lang.Integer", metaffi_int32_type)
		else if_class("[java.lang.Integer", metaffi_int32_array_type)
		
		else if_class("java.lang.Long", metaffi_int64_type)
		else if_class("[java.lang.Long", metaffi_int64_array_type)
		
		else if_class("java.lang.Boolean", metaffi_bool_type)
		else if_class("[java.lang.Boolean", metaffi_bool_array_type)
		
		else if_class("java.lang.Character", metaffi_char8_type)
		else if_class("[java.lang.Character", metaffi_char8_array_type)
		
		else if_class("Ljava.lang.String;", metaffi_string8_type)
		else if_class("[Ljava.lang.String;", metaffi_string8_array_type)
		
		else // handle
		{
			return metaffi_handle_type;
		}
		
	}
};
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
struct java_cdts_parse_callbacks : public cdts_parse_callbacks_interface
{
	JNIEnv* env = nullptr;
	
	explicit java_cdts_parse_callbacks(JNIEnv* env):env(env){}
	
	//--------------------------------------------------------------------
	template<typename T, typename jT>
	void set_numeric_to_jobject(jobjectArray objects, int index, const T& val, jclass cls, jmethodID meth, const std::function<jT(T)>& ConvFunc)
	{
		env->SetObjectArrayElement(objects, index, env->NewObject(cls, meth, ConvFunc(val)));
	}
	//--------------------------------------------------------------------
	template<typename T, typename char_t>
	void set_string_to_jobject(jobjectArray objects, int index, const T& val, const metaffi_size& size, const std::function<jstring(const char_t*, metaffi_size)>& ConvFunc)
	{
		jstring jstr = ConvFunc((char_t*)val, size);
		env->SetObjectArrayElement(objects, index, jstr);
	}
	//--------------------------------------------------------------------
	template<typename T, typename jT, typename jTArray>
	void set_numeric_array_to_jobject(jobjectArray objects, int cdts_index, const metaffi::runtime::numeric_n_array_wrapper<T>& arr_wrap,
	                                  const std::function<jTArray(jsize)>& new_array_t,
	                                  const std::function<void(jTArray, jsize, const jT*)>& set_array_t)
	{
		jTArray obj_array = new_array_t((jsize)arr_wrap.dimensions_lengths[0]);
		metaffi_size index[1] = { 0 };
		set_array_t(obj_array, (jsize)arr_wrap.dimensions_lengths[0], (jT*)arr_wrap.get_array_at(index, 1));
		env->SetObjectArrayElement(objects, cdts_index, obj_array);
	}
	//--------------------------------------------------------------------
	template<typename T, typename char_t>
	void set_string_array_to_jobject(jobjectArray values_to_set, int cdts_index, const metaffi::runtime::string_n_array_wrapper<T>& arr_wrap, const std::function<jstring(const char_t*, jsize)>& c_to_jstring)
	{
		metaffi_size size = arr_wrap.dimensions_lengths[0];
		if(size == 0){
			env->SetObjectArrayElement((jobjectArray)values_to_set, cdts_index, nullptr);
			return;
		}
		
		metaffi_size arrindex[1] = { 0 };
		jobjectArray res = env->NewObjectArray((jsize)size, cdts_java::string_class, nullptr);
		
		for(int i=0 ; i<size ; i++)
		{
			arrindex[0] = i;
			T curstr;
			metaffi_size curlen;
			arr_wrap.get_elem_at(arrindex, 1, &curstr, &curlen);
			jstring cur_jstr = c_to_jstring(curstr, curlen);
			env->SetObjectArrayElement(res, i, cur_jstr);
		}
		
		env->SetObjectArrayElement((jobjectArray)values_to_set, cdts_index, res);
	}
	//====================================================================
	template<typename T>
	void set_numeric_to_cdts(jobjectArray objectArray, int index, T& val, const std::function<T(jobject)>& jobject_to_c)
	{
		jobject val_to_set = env->GetObjectArrayElement(objectArray, index);
		val = jobject_to_c(val_to_set);
	}
	
	template<typename T, typename jarray_t>
	void set_numeric_array_to_cdts(jobjectArray objectArray, int index, T*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, const std::function<T*(jarray_t)>& jobject_to_c)
	{
		jarray_t val_to_set = (jarray_t)env->GetObjectArrayElement(objectArray, index);
		jsize len = env->GetArrayLength(val_to_set);
		if(len == 0)
		{
			arr = nullptr;
			dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
			dimensions_lengths[0] = 0;
			return;
		}
		
		dimensions = 1; // TODO: handle multi-dimensions
		dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		dimensions_lengths[0] = len;
		
		arr = jobject_to_c(val_to_set);
	}
	
	template<typename T, typename char_t>
	void set_string_to_cdts(jobject obj, int index, T& val, metaffi_size& s, const std::function<char_t*(jstring)>& jstring_to_c, const std::function<metaffi_size(jstring)>& jstring_size)
	{
		jstring curobj = (jstring)env->GetObjectArrayElement((jobjectArray)obj, index);
		
		val = jstring_to_c(curobj);
		s = jstring_size(curobj);
	}
	
	template<typename T, typename char_t>
	void set_string_array_to_cdts(jobject obj, int index, T*& arr, metaffi_size*& strings_lengths, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, const std::function<char_t*(jstring)>& jstring_to_c, const std::function<metaffi_size(jstring)>& jstring_size)
	{
		jobjectArray curarray = (jobjectArray)env->GetObjectArrayElement((jobjectArray)obj, index);
		
		jsize size = 0;
		if(!curarray || (size = env->GetArrayLength(curarray), size == 0))
		{
			arr = nullptr;
			dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
			dimensions_lengths[0] = 0;
		}
		
		dimensions = 1; // TODO: handle multi-dimensions
		dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
		dimensions_lengths[0] = size;
		
		strings_lengths = (metaffi_size*)malloc(sizeof(metaffi_size)*size);
		arr = (T*)malloc(sizeof(T)*size);
		
		for(int i=0 ; i<size ; i++)
		{
			jstring cur_item = (jstring)env->GetObjectArrayElement(curarray, i);
			
			arr[i] = jstring_to_c(cur_item);
			strings_lengths[i] = jstring_size(cur_item);
			
		}
	}
	
	void on_metaffi_float32(void* values_to_set, int index, const metaffi_float32& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_float32, jfloat>((jobjectArray)values_to_set, index, val_to_set, cdts_java::float_class, cdts_java::float_constructor,
		                                                [](metaffi_float32 v)->jfloat{ return (jfloat)v; } );
		
	}
	
	void on_metaffi_float32_array(void* values_to_set, int index, const metaffi_float32* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_float32> arr_wrap((metaffi_float32*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_float32, jfloat, jfloatArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                                   [this](jsize size)->jfloatArray{ return env->NewFloatArray(size); },
		                                                                   [this](jfloatArray jarray, jsize size, const jfloat* valarray)->void{ return env->SetFloatArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_float64(void* values_to_set, int index, const metaffi_float64& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_float64, jdouble>((jobjectArray)values_to_set, index, val_to_set, cdts_java::double_class, cdts_java::double_constructor,
		                                                 [](metaffi_float64 v)->jdouble{ return (jdouble)v; } );
		
		
	}
	
	void on_metaffi_float64_array(void* values_to_set, int index, const metaffi_float64* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_float64> arr_wrap((metaffi_float64*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_float64, jdouble, jdoubleArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                                     [this](jsize size)->jdoubleArray{ return env->NewDoubleArray(size); },
		                                                                     [this](jdoubleArray jarray, jsize size, const jdouble* valarray)->void{ return env->SetDoubleArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_int8(void* values_to_set, int index, const metaffi_int8& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_int8, jbyte>((jobjectArray)values_to_set, index, val_to_set, cdts_java::byte_class, cdts_java::byte_constructor,
		                                            [](metaffi_int8 v)->jbyte { return (jbyte)v; } );
		
	}
	
	void on_metaffi_int8_array(void* values_to_set, int index, const metaffi_int8* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_int8> arr_wrap((metaffi_int8*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_int8, jbyte, jbyteArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                              [this](jsize size)->jbyteArray{ return env->NewByteArray(size); },
		                                                              [this](jbyteArray jarray, jsize size, const jbyte* valarray)->void{ return env->SetByteArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_int16(void* values_to_set, int index, const metaffi_int16& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_int16, jshort>((jobjectArray)values_to_set, index, val_to_set, cdts_java::short_class, cdts_java::short_constructor,
		                                              [](metaffi_int16 v)->jshort { return (jshort)v; } );
		
	}
	
	void on_metaffi_int16_array(void* values_to_set, int index, const metaffi_int16* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_int16> arr_wrap((metaffi_int16*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_int16, jshort, jshortArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                                 [this](jsize size)->jshortArray{ return env->NewShortArray(size); },
		                                                                 [this](jshortArray jarray, jsize size, const jshort* valarray)->void{ return env->SetShortArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_int32(void* values_to_set, int index, const metaffi_int32& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_int32, jint>((jobjectArray)values_to_set, index, val_to_set, cdts_java::int_class, cdts_java::int_constructor,
		                                            [](metaffi_int32 v)->jint { return (jint)v; } );
		
	}
	
	void on_metaffi_int32_array(void* values_to_set, int index, const metaffi_int32* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_int32> arr_wrap((metaffi_int32*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_int32, jint, jintArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                             [this](jsize size)->jintArray{ return env->NewIntArray(size); },
		                                                             [this](jintArray jarray, jsize size, const jint* valarray)->void{ return env->SetIntArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_int64(void* values_to_set, int index, const metaffi_int64& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_int64, jlong>((jobjectArray)values_to_set, index, val_to_set, cdts_java::long_class, cdts_java::long_constructor,
		                                             [](metaffi_int64 v)->jlong { return (jlong)v; } );
		
	}
	
	void on_metaffi_int64_array(void* values_to_set, int index, const metaffi_int64* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_int64> arr_wrap((metaffi_int64*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_int64, jlong, jlongArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                               [this](jsize size)->jlongArray{ return env->NewLongArray(size); },
		                                                               [this](jlongArray jarray, jsize size, const jlong* valarray)->void{ return env->SetLongArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_uint8(void* values_to_set, int index, const metaffi_uint8& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_uint8, jbyte>((jobjectArray)values_to_set, index, val_to_set, cdts_java::byte_class, cdts_java::byte_constructor,
		                                             [](metaffi_int8 v)->jbyte { return (jbyte)v; } );
		
	}
	
	void on_metaffi_uint8_array(void* values_to_set, int index, const metaffi_uint8* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_uint8> arr_wrap((metaffi_uint8*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_uint8, jbyte, jbyteArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                               [this](jsize size)->jbyteArray{ return env->NewByteArray(size); },
		                                                               [this](jbyteArray jarray, jsize size, const jbyte* valarray)->void{ return env->SetByteArrayRegion(jarray, 0, size, valarray); });
		
		
	}
	
	void on_metaffi_uint16(void* values_to_set, int index, const metaffi_uint16& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_uint16, jshort>((jobjectArray)values_to_set, index, val_to_set, cdts_java::short_class, cdts_java::short_constructor,
		                                               [](metaffi_uint16 v)->jshort { return (jshort)v; } );
		
	}
	
	void on_metaffi_uint16_array(void* values_to_set, int index, const metaffi_uint16* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_uint16> arr_wrap((metaffi_uint16*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_uint16, jshort, jshortArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                                  [this](jsize size)->jshortArray{ return env->NewShortArray(size); },
		                                                                  [this](jshortArray jarray, jsize size, const jshort* valarray)->void{ return env->SetShortArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_uint32(void* values_to_set, int index, const metaffi_uint32& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_uint32, jint>((jobjectArray)values_to_set, index, val_to_set, cdts_java::int_class, cdts_java::int_constructor,
		                                             [](metaffi_uint32 v)->jint { return (jint)v; } );
		
	}
	
	void on_metaffi_uint32_array(void* values_to_set, int index, const metaffi_uint32* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_uint32> arr_wrap((metaffi_uint32*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_uint32, jint, jintArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                              [this](jsize size)->jintArray{ return env->NewIntArray(size); },
		                                                              [this](jintArray jarray, jsize size, const jint* valarray)->void{ return env->SetIntArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_uint64(void* values_to_set, int index, const metaffi_uint64& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_uint64, jlong>((jobjectArray)values_to_set, index, val_to_set, cdts_java::long_class, cdts_java::long_constructor,
		                                              [](metaffi_uint64 v)->jlong { return (jlong)v; } );
		
	}
	
	void on_metaffi_uint64_array(void* values_to_set, int index, const metaffi_uint64* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_uint64> arr_wrap((metaffi_uint64*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_uint64, jlong, jlongArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                                [this](jsize size)->jlongArray{ return env->NewLongArray(size); },
		                                                                [this](jlongArray jarray, jsize size, const jlong* valarray)->void{ return env->SetLongArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_bool(void* values_to_set, int index, const metaffi_bool& val_to_set) override
	{
		set_numeric_to_jobject<metaffi_bool, jboolean>((jobjectArray)values_to_set, index, val_to_set, cdts_java::boolean_class, cdts_java::boolean_constructor,
		                                               [](metaffi_bool v)->jboolean { return (jboolean)v; } );
		
	}
	
	void on_metaffi_bool_array(void* values_to_set, int index, const metaffi_bool* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_bool> arr_wrap((metaffi_bool*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_numeric_array_to_jobject<metaffi_bool, jboolean, jbooleanArray>((jobjectArray)values_to_set, index, arr_wrap,
		                                                                    [this](jsize size)->jbooleanArray{ return env->NewBooleanArray(size); },
		                                                                    [this](jbooleanArray jarray, jsize size, const jboolean* valarray)->void{ return env->SetBooleanArrayRegion(jarray, 0, size, valarray); });
		
	}
	
	void on_metaffi_handle(void* values_to_set, int index, const metaffi_handle& handle) override
	{
		jobject res = nullptr;
		if(handle != nullptr)
		{
			if(!openjdk_objects_table::instance().contains(jobject(handle))) // not java object, return MetaFFIHandle object
			{
				res = env->NewObject(cdts_java::metaffi_handle_class, cdts_java::metaffi_handle_constructor, (jlong)handle);
			}
			else
			{
				res = (jobject)handle;
			}
		}
		jsize len = env->GetArrayLength((jobjectArray)values_to_set);
		env->SetObjectArrayElement((jobjectArray)values_to_set, index, res);
	}
	
	void on_metaffi_handle_array(void* values_to_set, int index, const metaffi_handle* parray_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		numeric_n_array_wrapper<metaffi_handle> arr_wrap((metaffi_handle*)parray_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		metaffi_size size = arr_wrap.get_simple_array_length();
		if(size == 0){
			env->SetObjectArrayElement((jobjectArray)values_to_set, index, nullptr);
			return;
		}
		
		metaffi_size arrindex[1] = { 0 };
		metaffi_handle first_elem = arr_wrap.get_elem_at(arrindex, 1);
		jobjectArray res = env->NewObjectArray((jsize)size, cdts_java::object_class, nullptr);
		if(!openjdk_objects_table::instance().contains((jobject)first_elem)) // no java object, return MetaFFIHandle object
		{
			for(int i=0 ; i<size ; i++)
			{
				arrindex[0] = i;
				metaffi_handle cur_elem = arr_wrap.get_elem_at(arrindex, 1);
				jobject cur_obj = env->NewObject(cdts_java::metaffi_handle_class, cdts_java::metaffi_handle_constructor, (jlong)cur_elem);
				env->SetObjectArrayElement(res, i, cur_obj);
			}
		}
		else
		{
			for(int i=0 ; i<size ; i++)
			{
				arrindex[0] = i;
				metaffi_handle cur_elem = arr_wrap.get_elem_at(arrindex, 1);
				env->SetObjectArrayElement(res, i, (jobject)cur_elem);
			}
		}
		
		env->SetObjectArrayElement((jobjectArray)values_to_set, index, res);
		
	}
	
	void on_metaffi_string8(void* values_to_set, int index, metaffi_string8 const& val_to_set, const metaffi_size& val_length) override
	{
		set_string_to_jobject<metaffi_string8, char>((jobjectArray)values_to_set, index, val_to_set, val_length,
		                                             [this](const char* p, metaffi_size s)->jstring{
			// JNI does not provide sized conversion from UTF to jstring, as it relies on null-terminator.
			// thus, we copy to std::string. Another option is to set p[s]=0, but it is risky, as we might overwrite something.
			return env->NewStringUTF(std::string(p, s).c_str() );
		});
		
	}
	
	void on_metaffi_string8_array(void* values_to_set, int index, const metaffi_string8* parray_to_set, const metaffi_size* pelements_lengths_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		string_n_array_wrapper<metaffi_string8> arr_wrap((metaffi_string8*)parray_to_set, (metaffi_size*)pelements_lengths_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_string_array_to_jobject<metaffi_string8, char>((jobjectArray)values_to_set, index, arr_wrap,
		                                                   [this](const char* str, metaffi_size s)->jstring{ return env->NewStringUTF(std::string(str, s).c_str() ); });
	}
	
	void on_metaffi_string16(void* values_to_set, int index, metaffi_string16 const& val_to_set, const metaffi_size& val_length) override
	{
		set_string_to_jobject<metaffi_string16, char16_t>((jobjectArray)values_to_set, index, val_to_set, val_length,
		                                                  [this](const char16_t* p, metaffi_size s)->jstring{ return env->NewString((const jchar*)p, (jsize)s); });
		
	}
	
	void on_metaffi_string16_array(void* values_to_set, int index, const metaffi_string16* parray_to_set, const metaffi_size* pelements_lengths_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		string_n_array_wrapper<metaffi_string16> arr_wrap((metaffi_string16*)parray_to_set, (metaffi_size*)pelements_lengths_to_set, (metaffi_size*)parray_dimensions_lengths, (metaffi_size&)array_dimensions);
		set_string_array_to_jobject<metaffi_string16, char16_t>((jobjectArray)values_to_set, index, arr_wrap,
		                                                        [this](const char16_t* str, metaffi_size l)->jstring{ return env->NewString((jchar*)str, (jsize)l); });
		
	}
	
	void on_metaffi_string32(void* values_to_set, int index, metaffi_string32 const& val_to_set, const metaffi_size& val_length) override
	{
		throw std::runtime_error("Does not support UTF32");
	}
	
	void on_metaffi_string32_array(void* values_to_set, int index, const metaffi_string32* parray_to_set, const metaffi_size* pelements_lengths_to_set, const metaffi_size* parray_dimensions_lengths, const metaffi_size& array_dimensions) override
	{
		throw std::runtime_error("Does not support UTF32");
	}
	
};


jclass cdts_java::float_class = nullptr;
jmethodID cdts_java::float_constructor = nullptr;
jmethodID cdts_java::float_get_value = nullptr;
jclass cdts_java::float_array_class = nullptr;

jclass cdts_java::double_class = nullptr;
jmethodID cdts_java::double_constructor = nullptr;
jmethodID cdts_java::double_get_value = nullptr;
jclass cdts_java::double_array_class = nullptr;

jclass cdts_java::byte_class = nullptr;
jmethodID cdts_java::byte_constructor = nullptr;
jmethodID cdts_java::byte_get_value = nullptr;
jclass cdts_java::byte_array_class = nullptr;

jclass cdts_java::short_class = nullptr;
jmethodID cdts_java::short_constructor = nullptr;
jmethodID cdts_java::short_get_value = nullptr;
jclass cdts_java::short_array_class = nullptr;

jclass cdts_java::int_class = nullptr;
jmethodID cdts_java::int_constructor = nullptr;
jmethodID cdts_java::int_get_value = nullptr;
jclass cdts_java::int_array_class = nullptr;

jclass cdts_java::long_class = nullptr;
jmethodID cdts_java::long_constructor = nullptr;
jmethodID cdts_java::long_get_value = nullptr;
jclass cdts_java::long_array_class = nullptr;

jclass cdts_java::boolean_class = nullptr;
jmethodID cdts_java::boolean_constructor = nullptr;
jmethodID cdts_java::boolean_get_value = nullptr;
jclass cdts_java::boolean_array_class = nullptr;

jclass cdts_java::char_class = nullptr;
jmethodID cdts_java::char_constructor = nullptr;
jmethodID cdts_java::char_get_value = nullptr;
jclass cdts_java::char_array_class = nullptr;

jclass cdts_java::string_class = nullptr;
jmethodID cdts_java::string_constructor = nullptr;
jclass cdts_java::string_array_class = nullptr;

jclass cdts_java::object_class = nullptr;
jclass cdts_java::object_array_class = nullptr;

jclass cdts_java::metaffi_handle_class = nullptr;
jmethodID cdts_java::metaffi_handle_constructor = nullptr;
jmethodID cdts_java::metaffi_handle_get_value = nullptr;


cdts_java::cdts_java(cdt* cdts, metaffi_size cdts_length, JNIEnv* env): cdts(cdts, cdts_length), env(env)
{
}
//--------------------------------------------------------------------
cdt* cdts_java::get_cdts()
{
	return this->cdts.get_cdts();
}
//--------------------------------------------------------------------
jobjectArray cdts_java::parse()
{
	jobjectArray array = env->NewObjectArray((jsize)this->cdts.get_cdts_length(), cdts_java::object_class, nullptr);
	
	java_cdts_parse_callbacks pc(env);
	this->cdts.parse(array, pc);
	
	
	return array;
}
//--------------------------------------------------------------------
void cdts_java::build(jobjectArray parameters, metaffi_types_ptr parameters_types, int params_count, int starting_index)
{
	java_cdts_build_callbacks bc(env);
	this->cdts.build(parameters_types, params_count, parameters, starting_index, bc);
}
//--------------------------------------------------------------------
