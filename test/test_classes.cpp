#include <doctest/doctest.h>

#include "jvm_test_env.h"
#include "jvm_wrappers.h"

#include <string>
#include <vector>
#include <variant>

namespace
{
std::string take_variant_string(metaffi_variant& value)
{
	if(std::holds_alternative<metaffi_string8>(value))
	{
		return take_string8(std::get<metaffi_string8>(value));
	}
	return {};
}
}

TEST_CASE("class and field access")
{
	auto& env = jvm_test_env();

	auto some_ctor = env.guest_module.load_entity_with_info(
		"class=guest.SomeClass,callable=<init>",
		{make_type(metaffi_string8_type)},
		{make_alias_type(metaffi_handle_type, "guest.SomeClass")});
	auto [some_ptr] = some_ctor.call<cdt_metaffi_handle*>(std::string("meta"));
	JvmHandle some_handle(some_ptr);

	auto print = env.guest_module.load_entity(
		"class=guest.SomeClass,callable=print,instance_required",
		{metaffi_handle_type},
		{metaffi_string8_type});
	auto [printed] = print.call<std::string>(*some_handle.get());
	CHECK(printed == "Hello from SomeClass meta");

	auto get_name = env.guest_module.load_entity(
		"class=guest.SomeClass,callable=getName,instance_required",
		{metaffi_handle_type},
		{metaffi_string8_type});
	auto [name] = get_name.call<std::string>(*some_handle.get());
	CHECK(name == "meta");

	auto default_ctor = env.guest_module.load_entity_with_info(
		"class=guest.SomeClass,callable=<init>",
		{},
		{make_alias_type(metaffi_handle_type, "guest.SomeClass")});
	auto [default_ptr] = default_ctor.call<cdt_metaffi_handle*>();
	JvmHandle default_handle(default_ptr);
	auto [default_name] = get_name.call<std::string>(*default_handle.get());
	CHECK(default_name == "some");

	auto testmap_ctor = env.guest_module.load_entity_with_info(
		"class=guest.TestMap,callable=<init>",
		{},
		{make_alias_type(metaffi_handle_type, "guest.TestMap")});
	auto [map_ptr] = testmap_ctor.call<cdt_metaffi_handle*>();
	JvmHandle map_handle(map_ptr);

	auto map_set = env.guest_module.load_entity_with_info(
		"class=guest.TestMap,callable=set,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.TestMap"), make_type(metaffi_string8_type), make_type(metaffi_any_type)},
		{});
	CHECK_NOTHROW(map_set.call<>(*map_handle.get(), std::string("key"), std::string("value")));

	auto map_contains = env.guest_module.load_entity_with_info(
		"class=guest.TestMap,callable=contains,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.TestMap"), make_type(metaffi_string8_type)},
		{make_type(metaffi_bool_type)});
	auto [has_key] = map_contains.call<bool>(*map_handle.get(), std::string("key"));
	CHECK(has_key);

	auto map_get = env.guest_module.load_entity_with_info(
		"class=guest.TestMap,callable=get,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.TestMap"), make_type(metaffi_string8_type)},
		{make_type(metaffi_any_type)});
	auto [map_val] = map_get.call<metaffi_variant>(*map_handle.get(), std::string("key"));
	CHECK(take_variant_string(map_val) == "value");

	auto name_get = env.guest_module.load_entity_with_info(
		"class=guest.TestMap,field=name,getter,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.TestMap")},
		{make_type(metaffi_string8_type)});
	auto [field_name] = name_get.call<std::string>(*map_handle.get());
	CHECK(field_name == "name1");

	auto name_set = env.guest_module.load_entity_with_info(
		"class=guest.TestMap,field=name,setter,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.TestMap"), make_type(metaffi_string8_type)},
		{});
	CHECK_NOTHROW(name_set.call<>(*map_handle.get(), std::string("name2")));
	auto [field_name2] = name_get.call<std::string>(*map_handle.get());
	CHECK(field_name2 == "name2");
}

TEST_CASE("overloads and nested types")
{
	auto& env = jvm_test_env();

	auto ctor_default = env.guest_module.load_entity_with_info(
		"class=guest.OverloadExamples,callable=<init>",
		{},
		{make_alias_type(metaffi_handle_type, "guest.OverloadExamples")});
	auto [obj_default_ptr] = ctor_default.call<cdt_metaffi_handle*>();
	JvmHandle obj_default(obj_default_ptr);

	auto ctor_int = env.guest_module.load_entity_with_info(
		"class=guest.OverloadExamples,callable=<init>",
		{make_type(metaffi_int32_type)},
		{make_alias_type(metaffi_handle_type, "guest.OverloadExamples")});
	auto [obj_int_ptr] = ctor_int.call<cdt_metaffi_handle*>(7);
	JvmHandle obj_int(obj_int_ptr);

	auto get_value = env.guest_module.load_entity_with_info(
		"class=guest.OverloadExamples,callable=getValue,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.OverloadExamples")},
		{make_type(metaffi_int32_type)});
	auto [val_default] = get_value.call<int32_t>(*obj_default.get());
	auto [val_int] = get_value.call<int32_t>(*obj_int.get());
	CHECK(val_default == 0);
	CHECK(val_int == 7);

	auto add_int = env.guest_module.load_entity_with_info(
		"class=guest.OverloadExamples,callable=add,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.OverloadExamples"), make_type(metaffi_int32_type), make_type(metaffi_int32_type)},
		{make_type(metaffi_int32_type)});
	auto [sum_int] = add_int.call<int32_t>(*obj_int.get(), 2, 3);
	CHECK(sum_int == 5);

	auto add_double = env.guest_module.load_entity_with_info(
		"class=guest.OverloadExamples,callable=add,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.OverloadExamples"), make_type(metaffi_float64_type), make_type(metaffi_float64_type)},
		{make_type(metaffi_float64_type)});
	auto [sum_double] = add_double.call<double>(*obj_int.get(), 1.5, 2.5);
	CHECK(sum_double == doctest::Approx(4.0));

	auto add_string = env.guest_module.load_entity_with_info(
		"class=guest.OverloadExamples,callable=add,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.OverloadExamples"), make_type(metaffi_string8_type), make_type(metaffi_string8_type)},
		{make_type(metaffi_string8_type)});
	auto [sum_str] = add_string.call<std::string>(*obj_int.get(), std::string("a"), std::string("b"));
	CHECK(sum_str == "ab");

	auto make_inner = env.guest_module.load_entity_with_info(
		"class=guest.NestedTypes,callable=makeInner",
		{make_type(metaffi_int32_type)},
		{make_alias_type(metaffi_handle_type, "guest.NestedTypes$Inner")});
	auto [inner_ptr] = make_inner.call<cdt_metaffi_handle*>(9);
	JvmHandle inner_handle(inner_ptr);

	auto get_value_inner = env.guest_module.load_entity_with_info(
		"class=guest.NestedTypes$Inner,callable=getValue,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.NestedTypes$Inner")},
		{make_type(metaffi_int32_type)});
	auto [inner_val] = get_value_inner.call<int32_t>(*inner_handle.get());
	CHECK(inner_val == 9);
}

TEST_CASE("generic box and auto closeable")
{
	auto& env = jvm_test_env();

	auto box_ctor = env.guest_module.load_entity_with_info(
		"class=guest.GenericBox,callable=<init>",
		{make_type(metaffi_any_type)},
		{make_alias_type(metaffi_handle_type, "guest.GenericBox")});
	auto [box_ptr] = box_ctor.call<cdt_metaffi_handle*>(std::string("hello"));
	JvmHandle box_handle(box_ptr);

	auto box_get = env.guest_module.load_entity_with_info(
		"class=guest.GenericBox,callable=get,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.GenericBox")},
		{make_type(metaffi_any_type)});
	auto [box_val] = box_get.call<metaffi_variant>(*box_handle.get());
	CHECK(take_variant_string(box_val) == "hello");

	auto box_set = env.guest_module.load_entity_with_info(
		"class=guest.GenericBox,callable=set,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.GenericBox"), make_type(metaffi_any_type)},
		{});
	CHECK_NOTHROW(box_set.call<>(*box_handle.get(), std::string("world")));
	auto [box_val2] = box_get.call<metaffi_variant>(*box_handle.get());
	CHECK(take_variant_string(box_val2) == "world");

	auto resource_ctor = env.guest_module.load_entity_with_info(
		"class=guest.AutoCloseableResource,callable=<init>",
		{},
		{make_alias_type(metaffi_handle_type, "guest.AutoCloseableResource")});
	auto [res_ptr] = resource_ctor.call<cdt_metaffi_handle*>();
	JvmHandle res_handle(res_ptr);

	auto is_closed = env.guest_module.load_entity_with_info(
		"class=guest.AutoCloseableResource,callable=isClosed,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.AutoCloseableResource")},
		{make_type(metaffi_bool_type)});
	auto [closed_before] = is_closed.call<bool>(*res_handle.get());
	CHECK(!closed_before);

	auto close = env.guest_module.load_entity_with_info(
		"class=guest.AutoCloseableResource,callable=close,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.AutoCloseableResource")},
		{});
	CHECK_NOTHROW(close.call<>(*res_handle.get()));
	auto [closed_after] = is_closed.call<bool>(*res_handle.get());
	CHECK(closed_after);
}

TEST_CASE("static state and submodule")
{
	auto& env = jvm_test_env();

	auto five_seconds = env.guest_module.load_entity_with_info(
		"class=guest.StaticState,field=FIVE_SECONDS,getter",
		{},
		{make_type(metaffi_int64_type)});
	auto [five] = five_seconds.call<int64_t>();
	CHECK(five == 5);

	auto set_counter = env.guest_module.load_entity(
		"class=guest.StaticState,callable=setCounter",
		{metaffi_int32_type},
		{});
	CHECK_NOTHROW(set_counter.call<>(0));

	auto get_counter = env.guest_module.load_entity(
		"class=guest.StaticState,callable=getCounter",
		{},
		{metaffi_int32_type});
	auto [counter0] = get_counter.call<int32_t>();
	CHECK(counter0 == 0);

	auto inc_counter = env.guest_module.load_entity(
		"class=guest.StaticState,callable=incCounter",
		{metaffi_int32_type},
		{metaffi_int32_type});
	auto [counter1] = inc_counter.call<int32_t>(3);
	CHECK(counter1 == 3);

	auto [counter2] = get_counter.call<int32_t>();
	CHECK(counter2 == 3);

	auto sub_echo = env.guest_module.load_entity(
		"class=guest.sub.SubModule,callable=echo",
		{metaffi_string8_type},
		{metaffi_string8_type});
	auto [echo] = sub_echo.call<std::string>(std::string("ok"));
	CHECK(echo == "ok");
}
