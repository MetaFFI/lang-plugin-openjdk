#include <doctest/doctest.h>

#include "jvm_test_env.h"
#include "jvm_wrappers.h"

#include <string>
#include <vector>
#include <variant>

namespace
{
}

TEST_CASE("core functions")
{
	auto& env = jvm_test_env();
	trace_step("core functions: start");

	auto hello = env.guest_module.load_entity(
		"class=guest.CoreFunctions,callable=helloWorld",
		{},
		{metaffi_string8_type});
	trace_step("core functions: call helloWorld");
	auto [hello_msg] = hello.call<std::string>();
	trace_step("core functions: got helloWorld");
	CHECK(hello_msg == "Hello World, from Java");

	auto div = env.guest_module.load_entity(
		"class=guest.CoreFunctions,callable=divIntegers",
		{metaffi_int64_type, metaffi_int64_type},
		{metaffi_float64_type});
	trace_step("core functions: call divIntegers");
	auto [ratio] = div.call<double>(6LL, 4LL);
	trace_step("core functions: got divIntegers");
	CHECK(ratio == doctest::Approx(1.5));

	std::vector<metaffi::api::MetaFFITypeInfo> join_params = {
		make_array_type(metaffi_string8_array_type, 1)
	};
	std::vector<metaffi::api::MetaFFITypeInfo> join_ret = {
		make_type(metaffi_string8_type)
	};
	auto join_typed = env.guest_module.load_entity_with_info(
		"class=guest.CoreFunctions,callable=joinStrings",
		join_params,
		join_ret);
	std::vector<std::string> parts = {"a", "b", "c"};
	trace_step("core functions: call joinStrings");
	auto [joined] = join_typed.call<std::string>(parts);
	trace_step("core functions: got joinStrings");
	CHECK(joined == "a,b,c");

	auto wait = env.guest_module.load_entity(
		"class=guest.CoreFunctions,callable=waitABit",
		{metaffi_int64_type},
		{});
	trace_step("core functions: call waitABit");
	CHECK_NOTHROW(wait.call<>(5LL));
	trace_step("core functions: done waitABit");

	auto ret_null = env.guest_module.load_entity(
		"class=guest.CoreFunctions,callable=returnNull",
		{},
		{metaffi_any_type});
	trace_step("core functions: call returnNull");
	auto [null_val] = ret_null.call<metaffi_variant>();
	trace_step("core functions: got returnNull");
	CHECK(std::holds_alternative<cdt_metaffi_handle>(null_val));
	CHECK(std::get<cdt_metaffi_handle>(null_val).handle == nullptr);

	auto return_any_any = env.guest_module.load_entity_with_info(
		"class=guest.CoreFunctions,callable=returnAny",
		{make_type(metaffi_int32_type)},
		{make_type(metaffi_any_type)});
	auto return_any_list = env.guest_module.load_entity_with_info(
		"class=guest.CoreFunctions,callable=returnAny",
		{make_type(metaffi_int32_type)},
		{make_array_type(metaffi_string8_array_type, 1)});
	auto return_any_handle = env.guest_module.load_entity_with_info(
		"class=guest.CoreFunctions,callable=returnAny",
		{make_type(metaffi_int32_type)},
		{make_alias_type(metaffi_handle_type, "guest.SomeClass")});

	trace_step("core functions: call returnAny(0)");
	auto [any_int] = return_any_any.call<metaffi_variant>(0);
	trace_step("core functions: got returnAny(0)");
	CHECK(std::holds_alternative<metaffi_int32>(any_int));
	CHECK(std::get<metaffi_int32>(any_int) == 1);

	trace_step("core functions: call returnAny(1)");
	auto [any_str] = return_any_any.call<metaffi_variant>(1);
	trace_step("core functions: got returnAny(1)");
	CHECK(std::holds_alternative<metaffi_string8>(any_str));
	CHECK(take_string8(std::get<metaffi_string8>(any_str)) == "string");

	trace_step("core functions: call returnAny(2)");
	auto [any_double] = return_any_any.call<metaffi_variant>(2);
	trace_step("core functions: got returnAny(2)");
	CHECK(std::holds_alternative<metaffi_float64>(any_double));
	CHECK(std::get<metaffi_float64>(any_double) == doctest::Approx(3.0));

	trace_step("core functions: call returnAny(3)");
	auto [any_arr] = return_any_list.call<std::vector<std::string>>(3);
	trace_step("core functions: got returnAny(3)");
	REQUIRE(any_arr.size() == 3);
	CHECK(any_arr[0] == "list");
	CHECK(any_arr[1] == "of");
	CHECK(any_arr[2] == "strings");

	trace_step("core functions: call returnAny(4)");
	auto [class_ptr] = return_any_handle.call<cdt_metaffi_handle*>(4);
	trace_step("core functions: got returnAny(4)");
	JvmHandle class_handle(class_ptr);
	auto get_name = env.guest_module.load_entity(
		"class=guest.SomeClass,callable=getName,instance_required",
		{metaffi_handle_type},
		{metaffi_string8_type});
	auto [name] = get_name.call<std::string>(*class_handle.get());
	CHECK(name == "some");

	trace_step("core functions: call returnAny(99)");
	auto [any_null] = return_any_any.call<metaffi_variant>(99);
	trace_step("core functions: got returnAny(99)");
	CHECK(std::holds_alternative<cdt_metaffi_handle>(any_null));
	CHECK(std::get<cdt_metaffi_handle>(any_null).handle == nullptr);

	auto accepts_any = env.guest_module.load_entity(
		"class=guest.CoreFunctions,callable=acceptsAny",
		{metaffi_int32_type, metaffi_any_type},
		{});
	trace_step("core functions: call acceptsAny(0)");
	CHECK_NOTHROW(accepts_any.call<>(0, static_cast<int32_t>(5)));
	trace_step("core functions: call acceptsAny(1)");
	CHECK_NOTHROW(accepts_any.call<>(1, std::string("hello")));
	trace_step("core functions: call acceptsAny(2)");
	CHECK_NOTHROW(accepts_any.call<>(2, 3.5));
	trace_step("core functions: call acceptsAny(3)");
	{
		cdts null_params(2);
		null_params[0] = static_cast<metaffi_int32>(3);
		cdt_metaffi_handle null_handle{};
		null_handle.handle = nullptr;
		null_handle.runtime_id = 0;
		null_handle.release = nullptr;
		null_params[1].set_handle(&null_handle);
		CHECK_NOTHROW(accepts_any.call_raw(std::move(null_params)));
	}
	trace_step("core functions: call acceptsAny(4)");
	CHECK_NOTHROW(accepts_any.call<>(4, std::vector<int8_t>{1, 2, 3}));

	auto some_ctor = env.guest_module.load_entity_with_info(
		"class=guest.SomeClass,callable=<init>",
		{},
		{make_alias_type(metaffi_handle_type, "guest.SomeClass")});
	trace_step("core functions: call SomeClass ctor");
	auto [some_ptr] = some_ctor.call<cdt_metaffi_handle*>();
	JvmHandle some_handle(some_ptr);
	trace_step("core functions: call acceptsAny(5)");
	CHECK_NOTHROW(accepts_any.call<>(5, *some_handle.get()));
	trace_step("core functions: done");
}

TEST_CASE("primitive functions")
{
	auto& env = jvm_test_env();

	std::vector<metaffi::api::MetaFFITypeInfo> params = {
		make_type(metaffi_bool_type),
		make_type(metaffi_int8_type),
		make_type(metaffi_int16_type),
		make_type(metaffi_int32_type),
		make_type(metaffi_int64_type),
		make_type(metaffi_float32_type),
		make_type(metaffi_float64_type),
		make_type(metaffi_char8_type)
	};
	std::vector<metaffi::api::MetaFFITypeInfo> ret = {
		make_alias_type(metaffi_handle_type, "java.lang.Object[]")
	};
	auto accepts = env.guest_module.load_entity_with_info(
		"class=guest.PrimitiveFunctions,callable=acceptsPrimitives",
		params,
		ret);

	metaffi_char8 letter;
	letter = u8"a";
	auto [values_ptr] = accepts.call<cdt_metaffi_handle*>(true, (int8_t)1, (int16_t)2, (int32_t)3, (int64_t)4, 5.5f, 6.25, letter);
	JavaArray values{JvmHandle(values_ptr)};
	REQUIRE(values.length() == 8);
	CHECK(java_object_to_string(values.get_handle(0)) == "true");
	CHECK(java_object_to_string(values.get_handle(1)) == "1");
	CHECK(java_object_to_string(values.get_handle(2)) == "2");
	CHECK(java_object_to_string(values.get_handle(3)) == "3");
	CHECK(java_object_to_string(values.get_handle(4)) == "4");
	CHECK(java_object_to_string(values.get_handle(5)) == "5.5");
	CHECK(java_object_to_string(values.get_handle(6)) == "6.25");
	CHECK(java_object_to_string(values.get_handle(7)) == "a");

	std::vector<metaffi::api::MetaFFITypeInfo> bytes_param = {make_array_type(metaffi_int8_array_type, 1)};
	std::vector<metaffi::api::MetaFFITypeInfo> bytes_ret = {make_array_type(metaffi_int8_array_type, 1)};
	auto echo_bytes = env.guest_module.load_entity_with_info(
		"class=guest.PrimitiveFunctions,callable=echoBytes",
		bytes_param,
		bytes_ret);
	std::vector<int8_t> payload = {1, 2, 3};
	auto [echoed] = echo_bytes.call<std::vector<int8_t>>(payload);
	CHECK(echoed == payload);

	auto to_upper = env.guest_module.load_entity(
		"class=guest.PrimitiveFunctions,callable=toUpper",
		{metaffi_char8_type},
		{metaffi_char8_type});
	metaffi_char8 lower;
	lower = u8"b";
	auto [upper] = to_upper.call<metaffi_char8>(lower);
	CHECK(upper.c[0] == u8'B');
}

TEST_CASE("varargs functions")
{
	auto& env = jvm_test_env();

	std::vector<metaffi::api::MetaFFITypeInfo> sum_params = {make_array_type(metaffi_int32_array_type, 1)};
	auto sum = env.guest_module.load_entity_with_info(
		"class=guest.VarargsExamples,callable=sum",
		sum_params,
		{make_type(metaffi_int32_type)});
	std::vector<int32_t> values = {1, 2, 3, 4};
	auto [sum_val] = sum.call<int32_t>(values);
	CHECK(sum_val == 10);

	std::vector<metaffi::api::MetaFFITypeInfo> join_params = {
		make_type(metaffi_string8_type),
		make_array_type(metaffi_string8_array_type, 1)
	};
	auto join = env.guest_module.load_entity_with_info(
		"class=guest.VarargsExamples,callable=join",
		join_params,
		{make_type(metaffi_string8_type)});
	std::vector<std::string> pieces = {"b", "c"};
	auto [joined] = join.call<std::string>(std::string("a"), pieces);
	CHECK(joined == "a:b:c");
}

TEST_CASE("enum and optional")
{
	auto& env = jvm_test_env();

	auto get_color = env.guest_module.load_entity_with_info(
		"class=guest.EnumTypes,callable=getColor",
		{make_type(metaffi_int32_type)},
		{make_alias_type(metaffi_handle_type, "guest.EnumTypes$Color")});
	auto [color_ptr] = get_color.call<cdt_metaffi_handle*>(0);
	JvmHandle color_handle(color_ptr);

	auto color_name = env.guest_module.load_entity_with_info(
		"class=guest.EnumTypes,callable=colorName",
		{make_alias_type(metaffi_handle_type, "guest.EnumTypes$Color")},
		{make_type(metaffi_string8_type)});
	auto [color_str] = color_name.call<std::string>(*color_handle.get());
	CHECK(color_str == "RED");

	auto maybe_string = env.guest_module.load_entity(
		"class=guest.OptionalFunctions,callable=maybeString",
		{metaffi_bool_type},
		{metaffi_handle_type});
	auto [opt_ptr] = maybe_string.call<cdt_metaffi_handle*>(true);
	JvmHandle opt_handle(opt_ptr);

	auto is_present = env.guest_module.load_entity_with_info(
		"class=java.util.Optional,callable=isPresent,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.Optional")},
		{make_type(metaffi_bool_type)});
	auto [present] = is_present.call<bool>(*opt_handle.get());
	CHECK(present);

	auto get_opt = env.guest_module.load_entity_with_info(
		"class=java.util.Optional,callable=get,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.Optional")},
		{make_type(metaffi_string8_type)});
	auto [opt_val] = get_opt.call<std::string>(*opt_handle.get());
	CHECK(opt_val == "value");

	auto [opt_empty_ptr] = maybe_string.call<cdt_metaffi_handle*>(false);
	JvmHandle opt_empty(opt_empty_ptr);
	auto [present_empty] = is_present.call<bool>(*opt_empty.get());
	CHECK(!present_empty);
}
