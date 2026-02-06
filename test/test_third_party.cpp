#include <doctest/doctest.h>

#include "jvm_test_env.h"
#include "jvm_wrappers.h"

#include <string>
#include <vector>

namespace
{
std::string build_classpath(const std::string& left, const std::string& right)
{
	if(left.empty())
	{
		return right;
	}
	if(right.empty())
	{
		return left;
	}
	std::string result = left;
	result += jvm_classpath_separator();
	result += right;
	return result;
}
}

TEST_CASE("third party jars")
{
	auto& env = jvm_test_env();

	std::string third_party = require_env("METAFFI_JVM_THIRD_PARTY_CLASSPATH");
	std::string classpath = build_classpath(env.guest_classpath, third_party);
	metaffi::api::MetaFFIModule third_party_module(env.runtime.runtime_plugin(), classpath);

	auto get_logger = third_party_module.load_entity_with_info(
		"class=org.apache.logging.log4j.LogManager,callable=getLogger",
		{make_type(metaffi_string8_type)},
		{make_alias_type(metaffi_handle_type, "org.apache.logging.log4j.Logger")});
	auto [logger_ptr] = get_logger.call<cdt_metaffi_handle*>(std::string("MetaFFI"));
	JvmHandle logger_handle(logger_ptr);

	auto get_name = third_party_module.load_entity_with_info(
		"class=org.apache.logging.log4j.Logger,callable=getName,instance_required",
		{make_alias_type(metaffi_handle_type, "org.apache.logging.log4j.Logger")},
		{make_type(metaffi_string8_type)});
	auto [logger_name] = get_name.call<std::string>(*logger_handle.get());
	CHECK(logger_name == "MetaFFI");

	auto mapper_ctor = third_party_module.load_entity_with_info(
		"class=com.fasterxml.jackson.databind.ObjectMapper,callable=<init>",
		{},
		{make_alias_type(metaffi_handle_type, "com.fasterxml.jackson.databind.ObjectMapper")});
	auto [mapper_ptr] = mapper_ctor.call<cdt_metaffi_handle*>();
	JvmHandle mapper_handle(mapper_ptr);

	auto write_value = third_party_module.load_entity_with_info(
		"class=com.fasterxml.jackson.databind.ObjectMapper,callable=writeValueAsString,instance_required",
		{make_alias_type(metaffi_handle_type, "com.fasterxml.jackson.databind.ObjectMapper"), make_type(metaffi_any_type)},
		{make_type(metaffi_string8_type)});
	auto [json] = write_value.call<std::string>(*mapper_handle.get(), std::string("hello"));
	CHECK(json == "\"hello\"");

	auto string_ctor = third_party_module.load_entity_with_info(
		"class=java.lang.String,callable=<init>",
		{make_type(metaffi_string8_type)},
		{make_alias_type(metaffi_handle_type, "java.lang.String")});
	auto [string_ptr] = string_ctor.call<cdt_metaffi_handle*>(std::string("  "));
	JvmHandle string_handle(string_ptr);

	auto is_blank = third_party_module.load_entity_with_info(
		"class=org.apache.commons.lang3.StringUtils,callable=isBlank",
		{make_alias_type(metaffi_handle_type, "java.lang.CharSequence")},
		{make_type(metaffi_bool_type)});
	auto [blank] = is_blank.call<bool>(*string_handle.get());
	CHECK(blank);
}
