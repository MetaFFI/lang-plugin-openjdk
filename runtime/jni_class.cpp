#include "jni_class.h"
#include <sstream>
#include <utils/scope_guard.hpp>
#include "runtime_id.h"
#include "exception_macro.h"
#include "jbyte_wrapper.h"


//--------------------------------------------------------------------
jni_class::jni_class(JNIEnv* env, jclass cls) : env(env), cls(cls)
{}

//--------------------------------------------------------------------
jmethodID jni_class::load_method(const std::string& method_name, const argument_definition& return_type,
                                 const std::vector<argument_definition>& parameters_types, bool instance_required)
{
	std::stringstream ss;
	ss << "(";
	for(const argument_definition& a: parameters_types)
	{
		ss << a.to_jni_signature_type();
	}
	ss << ")";
	ss << (method_name == "<init>" ? "V" : return_type.to_jni_signature_type());
	
	std::string sig = ss.str();
	
	jmethodID id;
	if(!instance_required && method_name != "<init>")
	{
		id = env->GetStaticMethodID(cls, method_name.c_str(), sig.c_str());
	}
	else
	{
		id = env->GetMethodID(cls, method_name.c_str(), sig.c_str());
	}
	
	check_and_throw_jvm_exception(env, id);
	return id;
}

//--------------------------------------------------------------------
jfieldID
jni_class::load_field(const std::string& field_name, const argument_definition& field_type, bool instance_required)
{
	jfieldID id = !instance_required ? env->GetStaticFieldID(cls, field_name.c_str(),
	                                                         field_type.to_jni_signature_type().c_str()) :
	              env->GetFieldID(cls, field_name.c_str(), field_type.to_jni_signature_type().c_str());
	
	if(id == nullptr)
	{
		if(env->ExceptionCheck() == JNI_TRUE)
		{
			std::string err_msg =
					"failed getting field id: " + field_name + " of type: " + field_type.to_jni_signature_type() + "\n";
			err_msg += jvm::get_exception_description(env, env->ExceptionOccurred());
			throw std::runtime_error(err_msg);
		}
	}
	return id;
}

//--------------------------------------------------------------------
void jni_class::write_field_to_cdts(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id,
                                    const metaffi_type_info& t)
{
	jvalue val;
	bool is_static = obj == nullptr;
	switch(t.type)
	{
		case metaffi_bool_type:
			val.z = is_static ? env->GetStaticBooleanField(cls, field_id) : env->GetBooleanField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_int8_type:
			val.b = is_static ? env->GetStaticByteField(cls, field_id) : env->GetByteField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_uint16_type:
			val.c = is_static ? env->GetStaticCharField(cls, field_id) : env->GetCharField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_int16_type:
			val.s = is_static ? env->GetStaticShortField(cls, field_id) : env->GetShortField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_int32_type:
			val.i = is_static ? env->GetStaticIntField(cls, field_id) : env->GetIntField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_int64_type:
			val.j = is_static ? env->GetStaticLongField(cls, field_id) : env->GetLongField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_float32_type:
			val.f = is_static ? env->GetStaticFloatField(cls, field_id) : env->GetFloatField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_float64_type:
			val.d = is_static ? env->GetStaticDoubleField(cls, field_id) : env->GetDoubleField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			break;
		case metaffi_int8_array_type:
		case metaffi_bool_array_type:
		case metaffi_uint16_array_type:
		case metaffi_int16_array_type:
		case metaffi_int32_array_type:
		case metaffi_int64_array_type:
		case metaffi_float32_array_type:
		case metaffi_float64_array_type:
		case metaffi_handle_array_type:
		case metaffi_handle_type:
		case metaffi_string8_type:
		case metaffi_string16_type:
		case metaffi_string8_array_type:
		case metaffi_string16_array_type:
		case metaffi_any_type:
			val.l = is_static ? env->GetStaticObjectField(cls, field_id) : env->GetObjectField(obj, field_id);
			check_and_throw_jvm_exception(env, true);
			wrapper.from_jvalue(env, val, t, index);
			wrapper.switch_to_primitive(env, index);
			break;
		
		default:
			std::stringstream ss;
			ss << "unsupported field type: " << t.type;
			throw std::runtime_error(ss.str());
	}
	
}

//--------------------------------------------------------------------
void jni_class::write_cdts_to_field(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id)
{
	bool is_static = (obj == nullptr);

#define set_field(type, jvalue_field) \
    {                                  \
    if(is_static) \
        env->SetStatic##type##Field(cls, field_id, wrapper.to_jvalue(env, index).jvalue_field); \
    else \
        env->Set##type##Field(obj, field_id, wrapper.to_jvalue(env, index).jvalue_field);\
    }
	
	
	switch(wrapper[index]->type)
	{
		case metaffi_int8_type:
			set_field(Byte, b);
			break;
		case metaffi_int8_array_type:
			set_field(Object, l);
			break;
		case metaffi_int16_type:
			set_field(Short, s);
			break;
		case metaffi_int16_array_type:
			set_field(Object, l);
			break;
		case metaffi_int32_type:
			set_field(Int, i);
			break;
		case metaffi_int32_array_type:
			set_field(Object, l);
			break;
		case metaffi_int64_type:
			set_field(Long, j);
			break;
		case metaffi_int64_array_type:
			set_field(Object, l);
			break;
		case metaffi_uint8_type:
		case metaffi_uint8_array_type:
		case metaffi_uint16_type:
		case metaffi_uint16_array_type:
		case metaffi_uint32_type:
		case metaffi_uint32_array_type:
		case metaffi_uint64_type:
		{
			std::stringstream ss;
			ss << "The type " << wrapper[index]->type << " is not supported in JVM";
			throw std::runtime_error(ss.str());
		}
			break;
		case metaffi_float32_type:
			set_field(Float, f);
			break;
		case metaffi_float32_array_type:
			set_field(Object, l);
			break;
		case metaffi_float64_type:
			set_field(Double, d);
			break;
		case metaffi_float64_array_type:
			set_field(Object, l);
			break;
		case metaffi_bool_type:
			set_field(Boolean, z);
			break;
		case metaffi_bool_array_type:
			set_field(Object, l);
			break;
		case metaffi_handle_type:
			set_field(Object, l);
			break;
		case metaffi_handle_array_type:
			set_field(Object, l);
			break;
		case metaffi_string8_type:
			set_field(Object, l);
			break;
		case metaffi_string8_array_type:
			set_field(Object, l);
			break;
		case metaffi_string16_type:
			set_field(Object, l);
			break;
		case metaffi_string16_array_type:
			set_field(Object, l);
			break;
		case metaffi_string32_type:
			set_field(Object, l);
			break;
		case metaffi_string32_array_type:
			set_field(Object, l);
			break;
		case metaffi_callable_type:
			throw std::runtime_error("Not implemented yet");
			break;
		default:
			std::stringstream ss;
			ss << "Unsupported type to set field. Type: " << wrapper[index]->type;
			throw std::runtime_error(ss.str());
	}
	
	check_and_throw_jvm_exception(env, true);
}


//--------------------------------------------------------------------
void jni_class::call(const cdts_java_wrapper& params_wrapper, const cdts_java_wrapper& retval_wrapper,
                     const metaffi_type_info& retval_type, bool instance_required, bool is_constructor,
                     const std::set<uint8_t>& any_type_indices, jmethodID method)
{
	int start_index = instance_required ? 1 : 0;
	int params_length = params_wrapper.get_cdts_length() - start_index;
	std::vector<jvalue> args(params_length);
	
	for(int i = 0; i < params_length; ++i)
	{
		// if parameter expects a primitive in its object form - switch to object
		if(params_wrapper[i + start_index]->type != metaffi_handle_type &&
		   (params_wrapper[i + start_index]->type & metaffi_array_type) == 0 &&
		   any_type_indices.contains(i + start_index))
		{
			params_wrapper.switch_to_object(env, i + start_index);
		}
		
		args[i] = params_wrapper.to_jvalue(env, i + start_index);
	}
	
	if(!instance_required)
	{
		jvalue result;
		switch(retval_type.type)
		{
			case metaffi_int32_type:
				result.i = env->CallStaticIntMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_int64_type:
				result.j = env->CallStaticLongMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_int16_type:
				result.s = env->CallStaticShortMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_int8_type:
				result.b = env->CallStaticByteMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_float32_type:
				result.f = env->CallStaticFloatMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_float64_type:
				result.d = env->CallStaticDoubleMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_handle_type:
				result.l = is_constructor ? env->NewObject(cls, method, args.data()) : env->CallStaticObjectMethodA(cls,
				                                                                                                    method,
				                                                                                                    args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				retval_wrapper.switch_to_primitive(env, 0); // switch to metaffi primitive, if possible
				break;
			case metaffi_null_type:
				env->CallStaticVoidMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				break;
			case metaffi_string8_type:
				result.l = env->CallStaticObjectMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_uint8_array_type:
			case metaffi_int8_array_type:
			case metaffi_int16_array_type:
			case metaffi_int32_array_type:
			case metaffi_int64_array_type:
			case metaffi_float32_array_type:
			case metaffi_float64_array_type:
			case metaffi_bool_array_type:
			case metaffi_handle_array_type:
			case metaffi_string8_array_type:
			case metaffi_string16_array_type:
			{
				result.l = static_cast<jobjectArray>(env->CallStaticObjectMethodA(cls, method, args.data()));
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				env->DeleteLocalRef(result.l);
			}
				break;
			default:
				std::stringstream ss;
				ss << "Unsupported return type. Type: " << retval_type.type;
				throw std::runtime_error(ss.str());
		}
	}
	else
	{
		if(params_wrapper[0]->type != metaffi_handle_type)
		{
			throw std::runtime_error("expected an object in index 0");
		}
		
		if(params_wrapper[0]->cdt_val.metaffi_handle_val.runtime_id != OPENJDK_RUNTIME_ID)
		{
			throw std::runtime_error("expected Java object");
		}
		
		jobject obj = params_wrapper.to_jvalue(env, 0).l;
		
		jvalue result;
		switch(retval_type.type)
		{
			case metaffi_int32_type:
				result.i = env->CallIntMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_int64_type:
				result.j = env->CallLongMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_int16_type:
				result.s = env->CallShortMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_int8_type:
				result.b = env->CallByteMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_float32_type:
				result.f = env->CallFloatMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_float64_type:
				result.d = env->CallDoubleMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_any_type:
			case metaffi_handle_type:
				result.l = env->CallObjectMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				break;
			case metaffi_bool_type:
			{
				result.z = env->CallBooleanMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
			}
				break;
			case metaffi_uint8_array_type:
			case metaffi_int8_array_type:
			case metaffi_int16_array_type:
			case metaffi_int32_array_type:
			case metaffi_int64_array_type:
			case metaffi_float32_array_type:
			case metaffi_float64_array_type:
			case metaffi_bool_array_type:
			case metaffi_handle_array_type:
			case metaffi_string8_array_type:
			case metaffi_string16_array_type:
			{
				result.l = static_cast<jobject>(env->CallObjectMethodA(obj, method, args.data()));
				check_and_throw_jvm_exception(env, true);
				retval_wrapper.from_jvalue(env, result, retval_type, 0);
				env->DeleteLocalRef(result.l);
			}
				break;
			case metaffi_null_type:
				env->CallVoidMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(env, true);
				break;
			default:
				std::stringstream ss;
				ss << "Unsupported return type. Type: " << retval_type.type;
				throw std::runtime_error(ss.str());
		}
	}
}
//--------------------------------------------------------------------
