#include "jni_class.h"
#include <sstream>
#include <utils/scope_guard.hpp>
#include "runtime_id.h"

#define check_and_throw_jvm_exception(jvm_instance, env, var) \
if(env->ExceptionCheck() == JNI_TRUE)\
{\
std::string err_msg = jvm_instance->get_exception_description(env->ExceptionOccurred());\
throw std::runtime_error(err_msg);\
}\
else if(!var)\
{\
throw std::runtime_error("Failed to get " #var);\
}


//--------------------------------------------------------------------
jni_class::jni_class(std::shared_ptr<jvm> pjvm, JNIEnv* env, jclass cls):pjvm(pjvm), env(env),cls(cls){}

//--------------------------------------------------------------------
jmethodID jni_class::load_method(const std::string& method_name, const argument_definition& return_type, const std::vector<argument_definition>& parameters_types, bool instance_required)
{
	std::stringstream ss;
	ss << "(";
	for(const argument_definition& a : parameters_types){
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
	
	check_and_throw_jvm_exception(pjvm, env, id);
	return id;
}
//--------------------------------------------------------------------
jfieldID jni_class::load_field(const std::string& field_name, const argument_definition& field_type, bool instance_required)
{
	jfieldID id = !instance_required ? env->GetStaticFieldID(cls, field_name.c_str(), field_type.to_jni_signature_type().c_str()) :
	              env->GetFieldID(cls, field_name.c_str(), field_type.to_jni_signature_type().c_str());
	
	check_and_throw_jvm_exception(pjvm, env, id);
	return id;
}
//--------------------------------------------------------------------
void jni_class::write_field_to_cdts(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id, metaffi_type t)
{
	jvalue val;
	bool is_static = obj == nullptr;
	switch (t)
	{
		case metaffi_bool_type:
			val.z = is_static? env->GetStaticBooleanField(cls, field_id) : env->GetBooleanField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			break;
		case metaffi_int8_type:
			val.b = is_static? env->GetStaticByteField(cls, field_id) : env->GetByteField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			break;
		case metaffi_uint16_type:
			val.c = is_static? env->GetStaticCharField(cls, field_id) : env->GetCharField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			break;
		case metaffi_int16_type:
			val.s = is_static? env->GetStaticShortField(cls, field_id) : env->GetShortField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			break;
		case metaffi_int32_type:
			val.i = is_static? env->GetStaticIntField(cls, field_id) : env->GetIntField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			break;
		case metaffi_int64_type:
			val.j = is_static? env->GetStaticLongField(cls, field_id) : env->GetLongField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			break;
		case metaffi_float32_type:
			val.f = is_static? env->GetStaticFloatField(cls, field_id) : env->GetFloatField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			break;
		case metaffi_float64_type:
			val.d = is_static? env->GetStaticDoubleField(cls, field_id) : env->GetDoubleField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
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
			val.l = is_static? env->GetStaticObjectField(cls, field_id) : env->GetObjectField(obj, field_id);
			check_and_throw_jvm_exception(pjvm, env, true);
			wrapper.from_jvalue(pjvm, env, val, t, index);
			wrapper.switch_to_primitive(pjvm, env, index);
			break;
		
		default:
			std::stringstream ss;
			ss << "unsupported field type: " << t;
			throw std::runtime_error(ss.str());
	}
	
}
//--------------------------------------------------------------------
void jni_class::write_cdts_to_field(int index, cdts_java_wrapper& wrapper, jobject obj, jfieldID field_id)
{
	cdt* c = wrapper[index];
	bool is_static = (obj == nullptr);
	
	switch (c->type)
	{
		case metaffi_int8_type:
			if (is_static)
				env->SetStaticByteField(cls, field_id, c->cdt_val.metaffi_int8_val.val);
			else
				env->SetByteField(obj, field_id, c->cdt_val.metaffi_int8_val.val);
			break;
		case metaffi_int16_type:
			if (is_static)
				env->SetStaticShortField(cls, field_id, c->cdt_val.metaffi_int16_val.val);
			else
				env->SetShortField(obj, field_id, c->cdt_val.metaffi_int16_val.val);
			break;
		case metaffi_int32_type:
			if (is_static)
				env->SetStaticIntField(cls, field_id, c->cdt_val.metaffi_int32_val.val);
			else
				env->SetIntField(obj, field_id, c->cdt_val.metaffi_int32_val.val);
			break;
		case metaffi_int64_type:
			if (is_static)
				env->SetStaticLongField(cls, field_id, c->cdt_val.metaffi_int64_val.val);
			else
				env->SetLongField(obj, field_id, c->cdt_val.metaffi_int64_val.val);
			break;
		case metaffi_uint8_type:
		case metaffi_uint16_type:
		case metaffi_uint32_type:
		case metaffi_uint64_type:
		{
			const char *metaffi_type_str;
			metaffi_type_to_str(c->type, metaffi_type_str);
			std::stringstream ss;
			ss << "The type " << metaffi_type_str << " is not supported in JVM";
			throw std::runtime_error(ss.str());
		}break;
		case metaffi_float32_type:
			if (is_static)
				env->SetStaticFloatField(cls, field_id, c->cdt_val.metaffi_float32_val.val);
			else
				env->SetFloatField(obj, field_id, c->cdt_val.metaffi_float32_val.val);
			break;
		case metaffi_float64_type:
			if (is_static)
				env->SetStaticDoubleField(cls, field_id, c->cdt_val.metaffi_float64_val.val);
			else
				env->SetDoubleField(obj, field_id, c->cdt_val.metaffi_float64_val.val);
			break;
		case metaffi_bool_type:
			if (is_static)
				env->SetStaticBooleanField(cls, field_id, c->cdt_val.metaffi_bool_val.val);
			else
				env->SetBooleanField(obj, field_id, c->cdt_val.metaffi_bool_val.val);
			break;
		case metaffi_handle_type:
			if (is_static)
				env->SetStaticObjectField(cls, field_id, static_cast<jobject>(c->cdt_val.metaffi_handle_val.val));
			else
				env->SetObjectField(obj, field_id, static_cast<jobject>(c->cdt_val.metaffi_handle_val.val));
			break;
		case metaffi_string8_type:
		{
			jstring str = env->NewStringUTF(std::string(c->cdt_val.metaffi_string8_val.val,
			                                            c->cdt_val.metaffi_string8_val.length).c_str());
			
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, static_cast<jobject>(str));
			}
			else
			{
				env->SetObjectField(obj, field_id, static_cast<jobject>(str));
			}
			
			env->DeleteLocalRef(str);
		}break;
		
		case metaffi_int32_array_type:
		{
			if (c->cdt_val.metaffi_int32_array_val.dimensions != 1){
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jintArray arr = env->NewIntArray(c->cdt_val.metaffi_int32_array_val.dimensions_lengths[0]);
			env->SetIntArrayRegion(arr, 0, c->cdt_val.metaffi_int32_array_val.dimensions_lengths[0], reinterpret_cast<jint*>(c->cdt_val.metaffi_int32_array_val.vals));
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		} break;
		case metaffi_int64_array_type:
		{
			if (c->cdt_val.metaffi_int32_array_val.dimensions != 1){
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jlongArray arr = env->NewLongArray(c->cdt_val.metaffi_int64_array_val.dimensions_lengths[0]);
			env->SetLongArrayRegion(arr, 0, c->cdt_val.metaffi_int64_array_val.dimensions_lengths[0], reinterpret_cast<jlong*>(c->cdt_val.metaffi_int64_array_val.vals));
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		} break;
		case metaffi_int16_array_type:
		{
			if (c->cdt_val.metaffi_int32_array_val.dimensions != 1){
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jshortArray arr = env->NewShortArray(c->cdt_val.metaffi_int16_array_val.dimensions_lengths[0]);
			env->SetShortArrayRegion(arr, 0, c->cdt_val.metaffi_int16_array_val.dimensions_lengths[0], reinterpret_cast<jshort*>(c->cdt_val.metaffi_int16_array_val.vals));
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		} break;
		case metaffi_int8_array_type:
		{
			if (c->cdt_val.metaffi_int32_array_val.dimensions != 1){
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jbyteArray arr = env->NewByteArray(c->cdt_val.metaffi_int8_array_val.dimensions_lengths[0]);
			env->SetByteArrayRegion(arr, 0, c->cdt_val.metaffi_int8_array_val.dimensions_lengths[0], reinterpret_cast<jbyte*>(c->cdt_val.metaffi_int8_array_val.vals));
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		} break;
		case metaffi_float32_array_type:
		{
			if (c->cdt_val.metaffi_int32_array_val.dimensions != 1){
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jfloatArray arr = env->NewFloatArray(c->cdt_val.metaffi_float32_array_val.dimensions_lengths[0]);
			env->SetFloatArrayRegion(arr, 0, c->cdt_val.metaffi_float32_array_val.dimensions_lengths[0], reinterpret_cast<jfloat*>(c->cdt_val.metaffi_float32_array_val.vals));
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		} break;
		case metaffi_float64_array_type:
		{
			if (c->cdt_val.metaffi_int32_array_val.dimensions != 1){
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jdoubleArray arr = env->NewDoubleArray(c->cdt_val.metaffi_float64_array_val.dimensions_lengths[0]);
			env->SetDoubleArrayRegion(arr, 0, c->cdt_val.metaffi_float64_array_val.dimensions_lengths[0], reinterpret_cast<jdouble*>(c->cdt_val.metaffi_float64_array_val.vals));
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		} break;
		case metaffi_handle_array_type:
		{
			if (c->cdt_val.metaffi_int32_array_val.dimensions != 1){
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jobjectArray arr = env->NewObjectArray(c->cdt_val.metaffi_handle_array_val.dimensions_lengths[0], cls, nullptr);
			for (int i = 0; i < c->cdt_val.metaffi_handle_array_val.dimensions_lengths[0]; ++i)
			{
				env->SetObjectArrayElement(arr, i, reinterpret_cast<jobject>(c->cdt_val.metaffi_handle_array_val.vals[i]));
			}
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		} break;
		case metaffi_string8_array_type:
		{
			if (c->cdt_val.metaffi_string8_array_val.dimensions != 1)
			{
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jobjectArray arr = env->NewObjectArray(c->cdt_val.metaffi_string8_array_val.dimensions_lengths[0], env->FindClass("java/lang/String"), nullptr);
			for (int i = 0; i < c->cdt_val.metaffi_string8_array_val.dimensions_lengths[0]; i++)
			{
				jstring str = env->NewStringUTF(std::string(c->cdt_val.metaffi_string8_array_val.vals[i], c->cdt_val.metaffi_string8_array_val.vals_sizes[i]).c_str());
				env->SetObjectArrayElement(arr, i, str);
				env->DeleteLocalRef(str);
			}
			
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		}
		case metaffi_string16_array_type:
		{
			if (c->cdt_val.metaffi_string16_array_val.dimensions != 1)
			{
				throw std::runtime_error("Only arrays of 1 dimension are supported");
			}
			
			jobjectArray arr = env->NewObjectArray(c->cdt_val.metaffi_string16_array_val.dimensions_lengths[0], env->FindClass("java/lang/String"), nullptr);
			for (int i = 0; i < c->cdt_val.metaffi_string16_array_val.dimensions_lengths[0]; i++)
			{
				jstring str = env->NewString((const jchar*)c->cdt_val.metaffi_string16_array_val.vals[i], c->cdt_val.metaffi_string16_array_val.vals_sizes[i]);
				env->SetObjectArrayElement(arr, i, str);
				env->DeleteLocalRef(str);
			}
			
			if (is_static)
			{
				env->SetStaticObjectField(cls, field_id, arr);
			}
			else
			{
				env->SetObjectField(obj, field_id, arr);
			}
		}
		
		default:
			std::stringstream ss;
			ss << "Unsupported type to set field. Type: " << c->type;
			throw std::runtime_error(ss.str());
			
	}
	
	check_and_throw_jvm_exception(pjvm, env, true);
}
//--------------------------------------------------------------------
void jni_class::call(const cdts_java_wrapper& params_wrapper, const cdts_java_wrapper& retval_wrapper, metaffi_type retval_type, bool instance_required, bool is_constructor, const std::set<uint8_t>& any_type_indices, jmethodID method)
{
	int start_index = instance_required ? 1 : 0;
	int params_length = params_wrapper.get_cdts_length() - start_index;
	std::vector<jvalue> args(params_length);
	
	for (int i = 0; i < params_length; ++i)
	{
		// if parameter expects a primitive in its object form - switch to object
		if(params_wrapper[i+start_index]->type != metaffi_handle_type &&
				(params_wrapper[i+start_index]->type & metaffi_array_type) == 0 &&
					any_type_indices.contains(i+start_index))
		{
			params_wrapper.switch_to_object(pjvm, env, i+start_index);
		}
		
		args[i] = params_wrapper.to_jvalue(pjvm, env, i + start_index);
	}
	
	if(!instance_required)
	{
		jvalue result;
		switch (retval_type)
		{
			case metaffi_int32_type:
				result.i = env->CallStaticIntMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_int64_type:
				result.j = env->CallStaticLongMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_int16_type:
				result.s = env->CallStaticShortMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_int8_type:
				result.b = env->CallStaticByteMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_float32_type:
				result.f = env->CallStaticFloatMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_float64_type:
				result.d = env->CallStaticDoubleMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_handle_type:
				result.l = is_constructor ? env->NewObject(cls, method, args.data()) : env->CallStaticObjectMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				retval_wrapper.switch_to_primitive(pjvm, env, 0); // switch to metaffi primitive, if possible
				break;
			case metaffi_null_type:
				env->CallStaticVoidMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				break;
			case metaffi_string8_type:
				result.l = env->CallStaticObjectMethodA(cls, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
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
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				env->DeleteLocalRef(result.l);
			} break;
			default:
				std::stringstream ss;
				ss << "Unsupported return type. Type: " << retval_type;
				throw std::runtime_error(ss.str());
		}
	}
	else
	{
		if(params_wrapper[0]->type != metaffi_handle_type){
			throw std::runtime_error("expected an object in index 0");
		}
		
		if(params_wrapper[0]->cdt_val.metaffi_handle_val.runtime_id != OPENJDK_RUNTIME_ID){
			throw std::runtime_error("expected Java object");
		}
		
		jobject obj = params_wrapper.to_jvalue(pjvm, env, 0).l;
		
		jvalue result;
		switch (retval_type)
		{
			case metaffi_int32_type:
				result.i = env->CallIntMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_int64_type:
				result.j = env->CallLongMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_int16_type:
				result.s = env->CallShortMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_int8_type:
				result.b = env->CallByteMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_float32_type:
				result.f = env->CallFloatMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_float64_type:
				result.d = env->CallDoubleMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_any_type:
			case metaffi_handle_type:
				result.l = env->CallObjectMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				break;
			case metaffi_bool_type:
			{
				result.z = env->CallBooleanMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
			}	break;
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
				check_and_throw_jvm_exception(pjvm, env, true);
				retval_wrapper.from_jvalue(pjvm, env, result, retval_type, 0);
				env->DeleteLocalRef(result.l);
			} break;
			case metaffi_null_type:
				env->CallVoidMethodA(obj, method, args.data());
				check_and_throw_jvm_exception(pjvm, env, true);
				break;
			default:
				std::stringstream ss;
				ss << "Unsupported return type. Type: " << retval_type;
				throw std::runtime_error(ss.str());
		}
	}
}
//--------------------------------------------------------------------
void jni_class::validate_valid_jobject(jobject obj)
{
	jobjectRefType ref_type = env->GetObjectRefType(obj);
	
	if(ref_type == JNIWeakGlobalRefType){
		throw std::runtime_error("\"this\" has weak global ref type, meaning it might be garbage collected");
	}
	
	if(ref_type != JNILocalRefType && ref_type != JNIGlobalRefType){
		throw std::runtime_error("\"this\" has an invalid ref type");
	}
}
//--------------------------------------------------------------------
std::string jni_class::get_method_name(JNIEnv *env, jclass cls, jmethodID mid) const
{
	// Get the Class class
	jclass classClass = env->FindClass("java/lang/Class");
	check_and_throw_jvm_exception(pjvm, env, true);
	
	// Get the getDeclaredMethods method ID
	jmethodID midGetDeclaredMethods = env->GetMethodID(classClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
	check_and_throw_jvm_exception(pjvm, env, true);
	
	// Call getDeclaredMethods
	jobjectArray methods = (jobjectArray)env->CallObjectMethod(cls, midGetDeclaredMethods);
	check_and_throw_jvm_exception(pjvm, env, true);
	
	// Get the Method class
	jclass classMethod = env->FindClass("java/lang/reflect/Method");
	check_and_throw_jvm_exception(pjvm, env, true);
	
	// Get the getName method ID
	jmethodID midGetName = env->GetMethodID(classMethod, "getName", "()Ljava/lang/String;");
	check_and_throw_jvm_exception(pjvm, env, true);
	
	jsize methodCount = env->GetArrayLength(methods);
	check_and_throw_jvm_exception(pjvm, env, true);
	
	for (int i = 0; i < methodCount; i++)
	{
		jobject method = env->GetObjectArrayElement(methods, i);
		check_and_throw_jvm_exception(pjvm, env, true);
		
		// If the method IDs match, return the name
		if (env->FromReflectedMethod(method) == mid)
		{
			jstring name = (jstring)env->CallObjectMethod(method, midGetName);
			check_and_throw_jvm_exception(pjvm, env, true);
			
			const char* nameStr = env->GetStringUTFChars(name, nullptr);
			check_and_throw_jvm_exception(pjvm, env, true);
			
			std::string methodName(nameStr);  // Convert to std::string
			env->ReleaseStringUTFChars(name, nameStr);  // Release the C string
			return methodName;
		}
	}
	
	return "";  // Method not found
}
//--------------------------------------------------------------------
