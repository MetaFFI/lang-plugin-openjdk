#include <doctest/doctest.h>

#include "jvm_test_env.h"
#include "jvm_wrappers.h"

#include <string>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace
{
struct IntBinaryContext
{
	bool called = false;
};

void int_binary_callback(void* context, cdts* data, char** out_err)
{
	try
	{
		if(!context || !data)
		{
			throw std::runtime_error("Callback context is null");
		}
		auto& params = data[0];
		if(params.length != 2)
		{
			throw std::runtime_error("Expected 2 parameters");
		}
		auto* ctx = static_cast<IntBinaryContext*>(context);
		ctx->called = true;
		int32_t a = static_cast<int32_t>(params[0]);
		int32_t b = static_cast<int32_t>(params[1]);
		data[1][0] = static_cast<metaffi_int32>(a + b);
	}
	catch(const std::exception& ex)
	{
		set_callback_error(out_err, ex.what());
	}
}

struct StringLengthContext
{
	bool called = false;
};

void string_length_callback(void* context, cdts* data, char** out_err)
{
	try
	{
		if(!context || !data)
		{
			throw std::runtime_error("Callback context is null");
		}
		auto* ctx = static_cast<StringLengthContext*>(context);
		ctx->called = true;
		auto& params = data[0];
		if(params.length != 1)
		{
			throw std::runtime_error("Expected 1 parameter");
		}
		const char* value = reinterpret_cast<const char*>(params[0].cdt_val.string8_val);
		int32_t len = value ? static_cast<int32_t>(std::strlen(value)) : 0;
		data[1][0] = static_cast<metaffi_int32>(len);
	}
	catch(const std::exception& ex)
	{
		set_callback_error(out_err, ex.what());
	}
}

JvmHandle make_interface_proxy(const cdt_metaffi_callable& callable, const std::string& interface_name)
{
	auto& env = jvm_test_env();
	auto adapter = env.guest_module.load_entity_with_info(
		"class=metaffi.api.accessor.CallbackAdapters,callable=asInterface",
		{make_type(metaffi_callable_type), make_type(metaffi_string8_type)},
		{make_type(metaffi_handle_type)});
	auto [proxy_ptr] = adapter.call<cdt_metaffi_handle*>(callable, interface_name);
	return JvmHandle(proxy_ptr);
}
}

TEST_CASE("callbacks between C++ and Java")
{
	auto& env = jvm_test_env();
	trace_step("callbacks: start");

	IntBinaryContext add_ctx{};
	xcall add_xcall(reinterpret_cast<void*>(int_binary_callback), &add_ctx);
	std::vector<metaffi_type> add_params = {metaffi_int32_type, metaffi_int32_type};
	std::vector<metaffi_type> add_ret = {metaffi_int32_type};
	auto add_callable = make_callable(add_xcall, add_params, add_ret);

	JvmHandle add_proxy = make_interface_proxy(add_callable, "java.util.function.IntBinaryOperator");
	auto call_callback_add = env.guest_module.load_entity_with_info(
		"class=guest.CoreFunctions,callable=callCallbackAdd",
		{make_alias_type(metaffi_handle_type, "java.util.function.IntBinaryOperator")},
		{make_type(metaffi_int32_type)});
	trace_step("callbacks: call callCallbackAdd");
	auto [sum] = call_callback_add.call<int32_t>(*add_proxy.get());
	trace_step("callbacks: got callCallbackAdd");
	CHECK(sum == 3);
	CHECK(add_ctx.called);

	auto return_callback_add = env.guest_module.load_entity_with_info(
		"class=guest.CoreFunctions,callable=returnCallbackAdd",
		{},
		{make_alias_type(metaffi_handle_type, "java.util.function.IntBinaryOperator")});
	trace_step("callbacks: call returnCallbackAdd");
	auto [adder_ptr] = return_callback_add.call<cdt_metaffi_handle*>();
	trace_step("callbacks: got returnCallbackAdd");
	JvmHandle adder_handle(adder_ptr);
	auto apply_as_int = env.guest_module.load_entity_with_info(
		"class=java.util.function.IntBinaryOperator,callable=applyAsInt,instance_required",
		{make_alias_type(metaffi_handle_type, "java.util.function.IntBinaryOperator"),
		 make_type(metaffi_int32_type), make_type(metaffi_int32_type)},
		{make_type(metaffi_int32_type)});
	trace_step("callbacks: call applyAsInt");
	auto [adder_res] = apply_as_int.call<int32_t>(*adder_handle.get(), 4, 5);
	trace_step("callbacks: got applyAsInt");
	CHECK(adder_res == 9);

	auto return_transformer = env.guest_module.load_entity_with_info(
		"class=guest.Callbacks,callable=returnTransformer",
		{make_type(metaffi_string8_type)},
		{make_alias_type(metaffi_handle_type, "guest.Callbacks$StringTransformer")});
	trace_step("callbacks: call returnTransformer");
	auto [transformer_ptr] = return_transformer.call<cdt_metaffi_handle*>(std::string("?"));
	trace_step("callbacks: got returnTransformer");
	JvmHandle transformer_handle(transformer_ptr);

	auto call_transformer = env.guest_module.load_entity_with_info(
		"class=guest.Callbacks,callable=callTransformer",
		{make_alias_type(metaffi_handle_type, "guest.Callbacks$StringTransformer"), make_type(metaffi_string8_type)},
		{make_type(metaffi_string8_type)});
	trace_step("callbacks: call callTransformer");
	auto [transformed] = call_transformer.call<std::string>(*transformer_handle.get(), std::string("hi"));
	trace_step("callbacks: got callTransformer");
	CHECK(transformed == "hi?");
	auto transform = env.guest_module.load_entity_with_info(
		"class=guest.Callbacks$StringTransformer,callable=transform,instance_required",
		{make_alias_type(metaffi_handle_type, "guest.Callbacks$StringTransformer"), make_type(metaffi_string8_type)},
		{make_type(metaffi_string8_type)});
	trace_step("callbacks: call transform");
	auto [transformed2] = transform.call<std::string>(*transformer_handle.get(), std::string("ok"));
	trace_step("callbacks: got transform");
	CHECK(transformed2 == "ok?");

	StringLengthContext len_ctx{};
	xcall len_xcall(reinterpret_cast<void*>(string_length_callback), &len_ctx);
	std::vector<metaffi_type> len_params = {metaffi_string8_type};
	std::vector<metaffi_type> len_ret = {metaffi_int32_type};
	auto len_callable = make_callable(len_xcall, len_params, len_ret);
	trace_step("callbacks: create Function proxy");
	JvmHandle func_proxy = make_interface_proxy(len_callable, "java.util.function.Function");
	trace_step("callbacks: created Function proxy");

	auto call_function = env.guest_module.load_entity_with_info(
		"class=guest.Callbacks,callable=callFunction",
		{make_alias_type(metaffi_handle_type, "java.util.function.Function"), make_type(metaffi_string8_type)},
		{make_alias_type(metaffi_handle_type, "java.lang.Integer")});
	trace_step("callbacks: call callFunction");
	auto [func_result_ptr] = call_function.call<cdt_metaffi_handle*>(*func_proxy.get(), std::string("abcd"));
	trace_step("callbacks: got callFunction");
	JvmHandle func_result_handle(func_result_ptr);
	auto int_value = env.guest_module.load_entity_with_info(
		"class=java.lang.Integer,callable=intValue,instance_required",
		{make_alias_type(metaffi_handle_type, "java.lang.Integer")},
		{make_type(metaffi_int32_type)});
	trace_step("callbacks: call intValue");
	auto [len_val] = int_value.call<int32_t>(*func_result_handle.get());
	trace_step("callbacks: got intValue");
	CHECK(len_val == 4);
	CHECK(len_ctx.called);

	auto call_greeter = env.guest_module.load_entity_with_info(
		"class=guest.Interfaces,callable=callGreeter",
		{make_alias_type(metaffi_handle_type, "guest.Interfaces$Greeter"), make_type(metaffi_string8_type)},
		{make_type(metaffi_string8_type)});

	auto greeter_impl_ctor = env.guest_module.load_entity_with_info(
		"class=guest.Interfaces$SimpleGreeter,callable=<init>",
		{},
		{make_alias_type(metaffi_handle_type, "guest.Interfaces$SimpleGreeter")});
	trace_step("callbacks: call SimpleGreeter ctor");
	auto [impl_ptr] = greeter_impl_ctor.call<cdt_metaffi_handle*>();
	trace_step("callbacks: got SimpleGreeter ctor");
	JvmHandle impl_handle(impl_ptr);
	trace_step("callbacks: call callGreeter (impl)");
	auto [greet_msg2] = call_greeter.call<std::string>(*impl_handle.get(), std::string("World"));
	trace_step("callbacks: got callGreeter (impl)");
	CHECK(greet_msg2 == "Hello World");
}
