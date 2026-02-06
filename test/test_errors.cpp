#include <doctest/doctest.h>

#include "jvm_test_env.h"

TEST_CASE("error handling")
{
	auto& env = jvm_test_env();

	auto throw_runtime = env.guest_module.load_entity(
		"class=guest.Errors,callable=throwRuntime",
		{},
		{});
	CHECK_THROWS(throw_runtime.call<>());

	auto throw_checked = env.guest_module.load_entity(
		"class=guest.Errors,callable=throwChecked",
		{metaffi_bool_type},
		{});
	CHECK_NOTHROW(throw_checked.call<>(false));
	CHECK_THROWS(throw_checked.call<>(true));

	auto return_error = env.guest_module.load_entity(
		"class=guest.Errors,callable=returnErrorString",
		{metaffi_bool_type},
		{metaffi_string8_type});
	auto [ok] = return_error.call<std::string>(true);
	auto [err] = return_error.call<std::string>(false);
	CHECK(ok == "ok");
	CHECK(err == "error");

	auto returns_error = env.guest_module.load_entity(
		"class=guest.CoreFunctions,callable=returnsAnError",
		{},
		{});
	CHECK_THROWS(returns_error.call<>());
}
