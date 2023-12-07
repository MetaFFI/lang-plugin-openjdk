#include "argument_definition.h"
#include <sstream>

argument_definition::argument_definition(metaffi_type_with_alias type_alias)
{
	type = type_alias.type;
	if(type_alias.alias != nullptr){
		alias = std::string(type_alias.alias, type_alias.alias_length);
	}
}

std::string argument_definition::to_jni_signature_type() const
{
	std::stringstream ss;
	
	if(!alias.empty())
	{
		ss << "L"  << alias << ";";
		return ss.str();
	}
	
	if(type & metaffi_array_type){
		ss << "[";
	}
	
	metaffi_type tmp = type & (~metaffi_array_type);
	
	if (tmp == metaffi_null_type) ss << "V";
	else if (tmp == metaffi_bool_type) ss << "Z";
	else if (tmp == metaffi_int8_type || tmp == metaffi_uint8_type) ss << "B";
	else if (tmp == metaffi_char8_type) ss << "C";
	else if (tmp == metaffi_int16_type || tmp == metaffi_uint16_type) ss << "S";
	else if (tmp == metaffi_int32_type || tmp == metaffi_uint32_type) ss << "I";
	else if (tmp == metaffi_int64_type || tmp == metaffi_uint64_type) ss << "J";
	else if (tmp == metaffi_float32_type) ss << "F";
	else if (tmp == metaffi_float64_type) ss << "D";
	else if (tmp == metaffi_string8_type) ss << "Ljava/lang/String;";
	else if (tmp == metaffi_any_type) ss << "Ljava/lang/Object;";
	else if (tmp == metaffi_handle_type) ss << "Ljava/lang/Object;";
	
	return ss.str();
}