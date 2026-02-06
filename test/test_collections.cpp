#include <doctest/doctest.h>

#include "jvm_test_env.h"
#include "jvm_wrappers.h"

#include <string>
#include <vector>
#include <variant>
#include <type_traits>

namespace
{
std::string variant_to_string(metaffi_variant& value)
{
	return std::visit([](auto&& v) -> std::string
	{
		using V = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<V, metaffi_string8>)
		{
			return take_string8(v);
		}
		else if constexpr (std::is_same_v<V, metaffi_int8> || std::is_same_v<V, metaffi_uint8> ||
		                   std::is_same_v<V, metaffi_int16> || std::is_same_v<V, metaffi_uint16> ||
		                   std::is_same_v<V, metaffi_int32> || std::is_same_v<V, metaffi_uint32> ||
		                   std::is_same_v<V, metaffi_int64> || std::is_same_v<V, metaffi_uint64>)
		{
			return std::to_string(static_cast<long long>(v));
		}
		else if constexpr (std::is_same_v<V, metaffi_float32> || std::is_same_v<V, metaffi_float64>)
		{
			return std::to_string(static_cast<double>(v));
		}
		else
		{
			return {};
		}
	}, value);
}
}

TEST_CASE("collections and numbers")
{
	auto& env = jvm_test_env();

	auto make_list = env.guest_module.load_entity(
		"class=guest.CollectionFunctions,callable=makeStringList",
		{},
		{metaffi_handle_type});
	auto [list_ptr] = make_list.call<cdt_metaffi_handle*>();
	JvmHandle list_handle(list_ptr);

	auto list_size = env.guest_module.load_entity_with_info(
		"class=java.util.List,callable=size,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.List")},
		{make_type(metaffi_int32_type)});
	auto [size_val] = list_size.call<int32_t>(*list_handle.get());
	CHECK(size_val == 3);

	auto list_get_any = env.guest_module.load_entity_with_info(
		"class=java.util.List,callable=get,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.List"), make_type(metaffi_int32_type)},
		{make_type(metaffi_any_type)});
	auto [item0] = list_get_any.call<metaffi_variant>(*list_handle.get(), 0);
	auto [item1] = list_get_any.call<metaffi_variant>(*list_handle.get(), 1);
	auto [item2] = list_get_any.call<metaffi_variant>(*list_handle.get(), 2);
	CHECK(variant_to_string(item0) == "a");
	CHECK(variant_to_string(item1) == "b");
	CHECK(variant_to_string(item2) == "c");

	auto make_map = env.guest_module.load_entity(
		"class=guest.CollectionFunctions,callable=makeStringIntMap",
		{},
		{metaffi_handle_type});
	auto [map_ptr] = make_map.call<cdt_metaffi_handle*>();
	JvmHandle map_handle(map_ptr);

	auto map_get = env.guest_module.load_entity_with_info(
		"class=java.util.Map,callable=get,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.Map"), make_type(metaffi_any_type)},
		{make_type(metaffi_any_type)});
	auto [map_val] = map_get.call<metaffi_variant>(*map_handle.get(), std::string("a"));
	CHECK(variant_to_string(map_val) == "1");

	auto map_get_handle = env.guest_module.load_entity_with_info(
		"class=java.util.Map,callable=get,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.Map"), make_type(metaffi_any_type)},
		{make_type(metaffi_handle_type)});

	auto make_set = env.guest_module.load_entity(
		"class=guest.CollectionFunctions,callable=makeIntSet",
		{},
		{metaffi_handle_type});
	auto [set_ptr] = make_set.call<cdt_metaffi_handle*>();
	JvmHandle set_handle(set_ptr);

	auto set_contains = env.guest_module.load_entity_with_info(
		"class=java.util.Set,callable=contains,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.Set"), make_type(metaffi_any_type)},
		{make_type(metaffi_bool_type)});
	auto [contains_two] = set_contains.call<bool>(*set_handle.get(), static_cast<int32_t>(2));
	CHECK(contains_two);

	auto make_nested = env.guest_module.load_entity(
		"class=guest.CollectionFunctions,callable=makeNestedMap",
		{},
		{metaffi_handle_type});
	auto [nested_ptr] = make_nested.call<cdt_metaffi_handle*>();
	JvmHandle nested_handle(nested_ptr);
	auto [nested_list_ptr] = map_get_handle.call<cdt_metaffi_handle*>(*nested_handle.get(), std::string("nums"));
	JvmHandle nested_list_handle(nested_list_ptr);
	auto [nested_size] = list_size.call<int32_t>(*nested_list_handle.get());
	CHECK(nested_size == 3);
	auto [nested_item0] = list_get_any.call<metaffi_variant>(*nested_list_handle.get(), 0);
	CHECK(variant_to_string(nested_item0) == "1");

	auto make_some_list = env.guest_module.load_entity(
		"class=guest.CollectionFunctions,callable=makeSomeClassList",
		{},
		{metaffi_handle_type});
	auto [some_list_ptr] = make_some_list.call<cdt_metaffi_handle*>();
	JvmHandle some_list_handle(some_list_ptr);
	auto [some_list_size] = list_size.call<int32_t>(*some_list_handle.get());
	CHECK(some_list_size == 3);
	auto list_get_handle = env.guest_module.load_entity_with_info(
		"class=java.util.List,callable=get,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.List"), make_type(metaffi_int32_type)},
		{make_type(metaffi_handle_type)});
	auto [some0_ptr] = list_get_handle.call<cdt_metaffi_handle*>(*some_list_handle.get(), 0);
	JvmHandle some0_handle(some0_ptr);
	auto get_name = env.guest_module.load_entity(
		"class=guest.SomeClass,callable=getName,instance_required",
		{metaffi_handle_type},
		{metaffi_string8_type});
	auto [some0_name] = get_name.call<std::string>(*some0_handle.get());
	CHECK(some0_name == "a");

	auto big_int = env.guest_module.load_entity(
		"class=guest.CollectionFunctions,callable=bigIntegerValue",
		{},
		{metaffi_handle_type});
	auto [big_int_ptr] = big_int.call<cdt_metaffi_handle*>();
	JvmHandle big_int_handle(big_int_ptr);
	CHECK(java_object_to_string(big_int_handle) == "12345678901234567890");

	auto big_dec = env.guest_module.load_entity(
		"class=guest.CollectionFunctions,callable=bigDecimalValue",
		{},
		{metaffi_handle_type});
	auto [big_dec_ptr] = big_dec.call<cdt_metaffi_handle*>();
	JvmHandle big_dec_handle(big_dec_ptr);
	CHECK(java_object_to_string(big_dec_handle) == "12345.6789");
}
