#pragma once

#include "runtime/metaffi_primitives.h"
#include <string>

struct argument_definition
{
	metaffi_type type = metaffi_null_type;
	std::string alias;
	
	argument_definition() = default;
	explicit argument_definition(metaffi_type_info type_alias);
	
	[[nodiscard]] std::string to_jni_signature_type() const;
};
