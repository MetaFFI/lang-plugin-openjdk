#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define NOMINMAX
#include <doctest/doctest.h>
#include <runtime/runtime_plugin_api.h>
#include <filesystem>
#include "cdts_java_wrapper.h"
#include "runtime_id.h"
#include <utils/scope_guard.hpp>

std::filesystem::path g_module_path;
char* err = nullptr;

// make sure you include utils/scope_guard.hpp
// if_fail_code need to assume "char* err" is defined
#define jxcall_scope_guard(name, if_fail_code) \
	metaffi::utils::scope_guard sg_##name([&name]() \
	{ \
		if(name && name->is_valid()) \
		{ \
            char* err = nullptr;         \
			free_xcall(name, &err);   \
            if(err)                           \
            {                          \
                if_fail_code;  \
			}                                  \
                                              \
            name = nullptr;     \
		} \
	});

struct GlobalSetup
{
	GlobalSetup()
	{
		g_module_path = std::filesystem::path(__FILE__);
		g_module_path = g_module_path.parent_path();
		g_module_path.append("test");
		g_module_path.append("TestRuntime.class");
		
		err = nullptr;
		
		load_runtime(&err);
		if(err)
		{
			std::cerr << err << std::endl;
			exit(1);
		}
	}
	
	~GlobalSetup()
	{
		err = nullptr;
		
		free_runtime(&err);
		if(err)
		{
			std::cerr << err << std::endl;
			exit(2);
		}
	}
};

static GlobalSetup setup;


auto cppload_entity(const std::string& module_path,
						const std::string& function_path,
                        std::vector<metaffi_type_info> params_types,
						std::vector<metaffi_type_info> retvals_types)
{
	err = nullptr;
	
	metaffi_type_info* params_types_arr = params_types.empty() ? nullptr : params_types.data();
	metaffi_type_info* retvals_types_arr = retvals_types.empty() ? nullptr : retvals_types.data();
	
	xcall* pxcall = load_entity(module_path.c_str(),
									function_path.c_str(),
									params_types_arr, params_types.size(),
	                                retvals_types_arr, retvals_types.size(),
									&err);
	
	if(err)
	{
		FAIL(std::string(err));
	}
	
	REQUIRE((pxcall->pxcall_and_context[0] != nullptr));
	REQUIRE((pxcall->pxcall_and_context[1] != nullptr));
	
	return pxcall;
};

typedef void(*xcall_no_params_no_retvals)(void*,char**, uint64_t*);

TEST_SUITE("openjdk runtime api")
{
	TEST_CASE("runtime_test_target.hello_world")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=helloWorld";
		xcall* phello_world = cppload_entity(g_module_path.string(), function_path, {}, {});
		metaffi::utils::scope_guard sg([phello_world]()
		{
			char* err = nullptr;
			free_xcall(phello_world, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		(*phello_world)(&err);
		if(err){ FAIL(std::string(err)); }
	}

	TEST_CASE("runtime_test_target.returns_an_error")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=returnsAnError";
		xcall* preturns_an_error = cppload_entity(g_module_path.string(), function_path, {}, {});
		metaffi::utils::scope_guard sg([preturns_an_error]()
		{
			char* err = nullptr;
			free_xcall(preturns_an_error, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		(*preturns_an_error)(&err);
		REQUIRE((err != nullptr));
		free(err);
	}

	TEST_CASE("runtime_test_target.div_integers")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=divIntegers";
		std::vector<metaffi_type_info> params_types = { metaffi_type_info{metaffi_int32_type}, metaffi_type_info{metaffi_int32_type}};
		std::vector<metaffi_type_info> retvals_types = { metaffi_type_info{metaffi_float32_type} };

		xcall* pdiv_integers = cppload_entity(g_module_path.string(), function_path, params_types, retvals_types);
		metaffi::utils::scope_guard sg([pdiv_integers]()
		{
			char* err = nullptr;
			free_xcall(pdiv_integers, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		metaffi::utils::scope_guard sg2([pcdts]()
		{
			xllr_free_cdts_buffer(pcdts);
		});
		
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		pcdts_params[0].type = metaffi_int32_type;
		pcdts_params[0].cdt_val.int32_val = 10;
		pcdts_params[1].type = metaffi_int32_type;
		pcdts_params[1].cdt_val.int32_val = 2;
		
		(*pdiv_integers)(pcdts, &err);
		if(err){ FAIL(std::string(err)); }

		REQUIRE((pcdts_retvals[0].type == metaffi_float32_type));
		REQUIRE((pcdts_retvals[0].cdt_val.float32_val == 5.0));
	}

	TEST_CASE("runtime_test_target.join_strings")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=joinStrings";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_string8_array_type, nullptr, false, 1}};
		std::vector<metaffi_type_info> retvals_types = {metaffi_type_info{metaffi_string8_type}};

		xcall* join_strings = cppload_entity(g_module_path.string(), function_path, params_types, retvals_types);
		jxcall_scope_guard(join_strings, FAIL(std::string(err)));
		
		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		cdts_scope_guard(pcdts);
		
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		pcdts_params[0].set_new_array(3, 1, metaffi_string8_type);
		pcdts_params[0].cdt_val.array_val->arr[0].set_string((metaffi_string8)u8"one", false);
		pcdts_params[0].cdt_val.array_val->arr[1].set_string((metaffi_string8)u8"two", false);
		pcdts_params[0].cdt_val.array_val->arr[2].set_string((metaffi_string8)u8"three", false);

		(*join_strings)(pcdts, &err);
		if(err){ FAIL(std::string(err)); }

		REQUIRE((pcdts_retvals[0].type == metaffi_string8_type));
		REQUIRE((std::u8string(pcdts_retvals[0].cdt_val.string8_val) == u8"one,two,three"));
	}

	TEST_CASE("runtime_test_target.testmap.set_get_contains")
	{
		// create new testmap
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		std::vector<metaffi_type_info> retvals_types = {{metaffi_handle_type, (char*)"sanity/TestMap", false, 0}};

		xcall* pnew_testmap = cppload_entity(g_module_path.string(), function_path, {}, retvals_types);
		metaffi::utils::scope_guard sg([pnew_testmap]()
		{
			char* err = nullptr;
			free_xcall(pnew_testmap, &err);
			if(err){ FAIL(std::string(err)); }
		});

		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg2([pcdts]()
		{
			xllr_free_cdts_buffer(pcdts);
		});
		cdts& params_cdts = pcdts[0];
		cdts& retvals_cdts = pcdts[1];

		(*pnew_testmap)((cdts*)pcdts, &err);
		if(err){ FAIL(std::string(err)); }
		
		REQUIRE((retvals_cdts[0].type == metaffi_handle_type));
		REQUIRE((retvals_cdts[0].cdt_val.handle_val->handle != nullptr));
		REQUIRE((retvals_cdts[0].cdt_val.handle_val->runtime_id == OPENJDK_RUNTIME_ID));

		cdt_metaffi_handle* testmap_instance = retvals_cdts[0].cdt_val.handle_val;

		// set
		function_path = "class=sanity.TestMap,callable=set,instance_required";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}, metaffi_type_info{metaffi_any_type}};

		xcall* p_testmap_set = cppload_entity(g_module_path.string(), function_path, params_types, {});
		metaffi::utils::scope_guard sg3([p_testmap_set]()
		{
			char* err = nullptr;
			free_xcall(p_testmap_set, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts2 = (cdts*)xllr_alloc_cdts_buffer(3, 0);
		metaffi::utils::scope_guard sg10([pcdts2]()
        {
            xllr_free_cdts_buffer(pcdts2);
        });
		cdts& params_cdts2 = pcdts2[0];
		
		params_cdts2[0].set_handle(testmap_instance);
		params_cdts2[1].set_string((metaffi_string8)std::u8string(u8"key").c_str(), true);
		params_cdts2[2] = ((int32_t)42);

		(*p_testmap_set)(pcdts2, &err);
		if(err){ FAIL(std::string(err)); }
		
		// contains
		function_path = "class=sanity.TestMap,callable=contains,instance_required";
		std::vector<metaffi_type_info> params_contains_types = { metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type} };
		std::vector<metaffi_type_info> retvals_contains_types = { metaffi_type_info{metaffi_bool_type} };

		xcall* p_testmap_contains = cppload_entity(g_module_path.string(), function_path, params_contains_types, retvals_contains_types);
		metaffi::utils::scope_guard sg4([p_testmap_contains]()
		{
			char* err = nullptr;
			free_xcall(p_testmap_contains, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts3 = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		metaffi::utils::scope_guard sg11([pcdts3]()
        {
            xllr_free_cdts_buffer(pcdts3);
        });
		cdts& params_cdts3 = pcdts3[0];
		cdts& retvals_cdts3 = pcdts3[1];
		
		params_cdts3[0].set_handle(testmap_instance);
		params_cdts3[1].set_string((metaffi_string8)u8"key", true);

		(*p_testmap_contains)(pcdts3, &err);
		if(err){ FAIL(std::string(err)); }
		
		REQUIRE((retvals_cdts3[0].type == metaffi_bool_type));
		REQUIRE((retvals_cdts3[0].cdt_val.bool_val != 0));
		
		// get
		function_path = "class=sanity.TestMap,callable=get,instance_required";
		std::vector<metaffi_type_info> params_get_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};
		std::vector<metaffi_type_info> retvals_get_types = {metaffi_type_info{metaffi_any_type}};

		xcall* p_testmap_get = cppload_entity(g_module_path.string(), function_path, params_get_types, retvals_get_types);
		metaffi::utils::scope_guard sg5([p_testmap_get]()
		{
			char* err = nullptr;
			free_xcall(p_testmap_get, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts4 = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		metaffi::utils::scope_guard sg12([pcdts4]()
        {
            xllr_free_cdts_buffer(pcdts4);
        });
		cdts& params_cdts4 = pcdts4[0];
		cdts& retvals_cdts4 = pcdts4[1];
		
		params_cdts4[0].set_handle(testmap_instance);
		params_cdts4[1].set_string((char8_t*)u8"key", true);

 		(*p_testmap_get)((cdts*)pcdts4, &err);
		if(err){ FAIL(std::string(err)); }
		
		REQUIRE((retvals_cdts4[0].type == metaffi_int32_type));
		REQUIRE((retvals_cdts4[0].cdt_val.int32_val == 42));
	}

	TEST_CASE("runtime_test_target.testmap.set_get_contains_cpp_object")
	{
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		std::vector<metaffi_type_info> retvals_types = {{metaffi_handle_type, (char*)"sanity/TestMap", false, 0}};
		
		xcall* pnew_testmap = cppload_entity(g_module_path.string(), function_path, {}, retvals_types);
		metaffi::utils::scope_guard sg([pnew_testmap]()
		{
			char* err = nullptr;
			free_xcall(pnew_testmap, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg2([pcdts]()
		{
			xllr_free_cdts_buffer(pcdts);
		});
		
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		
		(*pnew_testmap)(pcdts, &err);
		if(err){ FAIL(std::string(err)); }
		
		REQUIRE((pcdts_retvals[0].type == metaffi_handle_type));
		REQUIRE((pcdts_retvals[0].cdt_val.handle_val->handle != nullptr));
		REQUIRE((pcdts_retvals[0].cdt_val.handle_val->runtime_id == OPENJDK_RUNTIME_ID));
		
		cdt_metaffi_handle* testmap_instance = pcdts_retvals[0].cdt_val.handle_val;
		
		// set
		function_path = "class=sanity.TestMap,callable=set,instance_required";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}, metaffi_type_info{metaffi_any_type}};
		
		xcall* p_testmap_set = cppload_entity(g_module_path.string(), function_path, params_types, {});
		metaffi::utils::scope_guard sg3([p_testmap_set]()
		{
			char* err = nullptr;
			free_xcall(p_testmap_set, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts2 = (cdts*)xllr_alloc_cdts_buffer(3, 0);
		metaffi::utils::scope_guard sg4([pcdts2]()
		{
			xllr_free_cdts_buffer(pcdts2);
		});
		
		cdts& pcdts_params2 = pcdts2[0];
		cdts& pcdts_retvals2 = pcdts2[1];
		
		std::vector<int> vec_to_insert = {1,2,3};
		
		pcdts_params2[0].set_handle(testmap_instance);
		pcdts_params2[1].set_string((metaffi_string8)u8"key", true);
		pcdts_params2[2].set_handle(new cdt_metaffi_handle{&vec_to_insert, 733, nullptr});

		(*p_testmap_set)((cdts*)pcdts2, &err);
		if(err){ FAIL(std::string(err)); }
		
		// contains
		function_path = "class=sanity.TestMap,callable=contains,instance_required";
		std::vector<metaffi_type_info> params_contains_types = { metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type} };
		std::vector<metaffi_type_info> retvals_contains_types = { metaffi_type_info{metaffi_bool_type} };
		
		xcall* p_testmap_contains = cppload_entity(g_module_path.string(), function_path, params_contains_types, retvals_contains_types);
		metaffi::utils::scope_guard sg5([p_testmap_contains]()
		{
			char* err = nullptr;
			free_xcall(p_testmap_contains, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts5 = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		cdts_scope_guard(pcdts5);
		cdts& pcdts_params5 = pcdts5[0];
		cdts& pcdts_retvals5 = pcdts5[1];
		
		pcdts_params5[0].set_handle(testmap_instance);
		pcdts_params5[1].set_string((metaffi_string8)u8"key", true);
		
		(*p_testmap_contains)(pcdts5, &err);
		if(err){ FAIL(std::string(err)); }
		
		REQUIRE((pcdts_retvals5[0].type == metaffi_bool_type));
		REQUIRE((pcdts_retvals5[0].cdt_val.bool_val != 0));
		
		// get
		function_path = "class=sanity.TestMap,callable=get,instance_required";
		std::vector<metaffi_type_info> params_get_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};
		std::vector<metaffi_type_info> retvals_get_types = {metaffi_type_info{metaffi_any_type}};
		
		xcall* p_testmap_get = cppload_entity(g_module_path.string(), function_path, params_get_types, retvals_get_types);
		metaffi::utils::scope_guard sg6([p_testmap_get]()
		{
			char* err = nullptr;
			free_xcall(p_testmap_get, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts4 = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		cdts_scope_guard(pcdts4);
		cdts& pcdts_params4 = pcdts4[0];
		cdts& pcdts_retvals4 = pcdts4[1];
		
		pcdts_params4[0].set_handle(testmap_instance);
		pcdts_params4[1].set_string((char8_t*)u8"key", true);
		
		(*p_testmap_get)(pcdts4, &err);
		if(err){ FAIL(std::string(err)); }
		
		
		REQUIRE((pcdts_retvals4[0].type == metaffi_handle_type));
		auto& vector_pulled = *(std::vector<int>*)pcdts_retvals4[0].cdt_val.handle_val->handle;

		REQUIRE((vector_pulled[0] == 1));
		REQUIRE((vector_pulled[1] == 2));
		REQUIRE((vector_pulled[2] == 3));

	}

	TEST_CASE("runtime_test_target.testmap.get_set_name")
	{
		// create new testmap
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		std::vector<metaffi_type_info> retvals_types = {{metaffi_handle_type, (char*)"sanity/TestMap", false, 0}};
		
		xcall* pnew_testmap = cppload_entity(g_module_path.string(), function_path, {}, retvals_types);
		metaffi::utils::scope_guard sg([pnew_testmap]()
		{
			char* err = nullptr;
			free_xcall(pnew_testmap, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		
		(*pnew_testmap)(pcdts, &err);
		if(err){ FAIL(std::string(err)); }
		
		
		REQUIRE((pcdts_retvals[0].type == metaffi_handle_type));
		REQUIRE((pcdts_retvals[0].cdt_val.handle_val->handle != nullptr));
		REQUIRE((pcdts_retvals[0].cdt_val.handle_val->runtime_id == OPENJDK_RUNTIME_ID));
		
		cdt_metaffi_handle* testmap_instance = pcdts_retvals[0].cdt_val.handle_val;
		
		
		// load getter
		function_path = "class=sanity.TestMap,field=name,instance_required,getter";
		std::vector<metaffi_type_info> params_name_getter_types = {metaffi_type_info{metaffi_handle_type}};
		std::vector<metaffi_type_info> retvals_name_getter_types = {metaffi_type_info{metaffi_string8_type}};
		
		xcall* pget_name = cppload_entity(g_module_path.string(), function_path, params_name_getter_types, retvals_name_getter_types);
		metaffi::utils::scope_guard sg2([pget_name]()
		{
			char* err = nullptr;
			free_xcall(pget_name, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		// load setter
		function_path = "class=sanity.TestMap,field=name,instance_required,setter";
		std::vector<metaffi_type_info> params_name_setter_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};
		
		xcall* pset_name = cppload_entity(g_module_path.string(), function_path, params_name_setter_types, {});
		metaffi::utils::scope_guard sg3([pset_name]()
		{
			char* err = nullptr;
			free_xcall(pset_name, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
		// get name
		cdts* pcdts3 = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		cdts& pcdts_params3 = pcdts3[0];
		cdts& pcdts_retvals3 = pcdts3[1];
		
		pcdts_params3[0].set_handle(testmap_instance);
		
		(*pget_name)(pcdts3, &err);
		if(err){ FAIL(std::string(err)); }
		
		
		REQUIRE((pcdts_retvals3[0].type == metaffi_string8_type));
		REQUIRE((std::u8string(pcdts_retvals3[0].cdt_val.string8_val).empty()));
		
		// set name to "name is my name"
		cdts* pcdts4 = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg4([pcdts4]()
		{
			xllr_free_cdts_buffer(pcdts4);
		});
		
		cdts& pcdts_params4 = pcdts4[0];
		cdts& pcdts_retvals4 = pcdts4[1];
		
		pcdts_params4[0].set_handle(testmap_instance);
		pcdts_params4[1].set_string((metaffi_string8)u8"name is my name", true);
		
		(*pset_name)(pcdts4, &err);
		if(err){ FAIL(std::string(err)); }
		
		
		// get name again and make sure it is "name is my name"
		cdts* pcdts5 = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		cdts_scope_guard(pcdts5);
		
		cdts& pcdts_params5 = pcdts5[0];
		cdts& pcdts_retvals5 = pcdts5[1];
		
		pcdts_params5[0].set_handle(testmap_instance);
		
		(*pget_name)(pcdts5, &err);
		if(err){ FAIL(std::string(err)); }
		
		REQUIRE((pcdts_retvals5[0].type == metaffi_string8_type));
		REQUIRE((std::u8string(pcdts_retvals5[0].cdt_val.string8_val) == u8"name is my name"));
	}

	TEST_CASE("runtime_test_target.wait_a_bit")
	{
	    // get five_seconds global
	    std::string function_path = "class=sanity.TestRuntime,field=fiveSeconds,getter";
	    std::vector<metaffi_type_info> retvals_fiveSeconds_getter_types = {{metaffi_int32_type, nullptr, false, 0}};
	
	    xcall* pfive_seconds_getter = cppload_entity(g_module_path.string(), function_path, {}, retvals_fiveSeconds_getter_types);
		metaffi::utils::scope_guard sg([pfive_seconds_getter]()
		{
			char* err = nullptr;
			free_xcall(pfive_seconds_getter, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
	    cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg2([pcdts]()
		{
			xllr_free_cdts_buffer(pcdts);
		});

	    cdts& pcdts_params = pcdts[0];
	    cdts& pcdts_retvals = pcdts[1];
	
	    (*pfive_seconds_getter)(pcdts, &err);
	    if(err){ FAIL(std::string(err)); }
	    
	
	    REQUIRE((pcdts_retvals[0].type == metaffi_int32_type));
	    REQUIRE((pcdts_retvals[0].cdt_val.int32_val == 5));
	
	    int32_t five = pcdts_retvals[0].cdt_val.int32_val;
	
	    // call wait_a_bit
	    function_path = "class=sanity.TestRuntime,callable=waitABit";
	    std::vector<metaffi_type_info> params_waitABit_types = {metaffi_type_info{metaffi_int32_type}};
	
	    xcall* pwait_a_bit = cppload_entity(g_module_path.string(), function_path, params_waitABit_types, {});
		metaffi::utils::scope_guard sg3([pwait_a_bit]()
		{
			char* err = nullptr;
			free_xcall(pwait_a_bit, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
	    cdts* pcdts2 = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		cdts_scope_guard(pcdts2);
	    cdts& pcdts_params2 = pcdts2[0];
	    cdts& pcdts_retvals2 = pcdts2[1];
	
		pcdts_params2[0] = five;
	
	    (*pwait_a_bit)(pcdts2, &err);
	    if(err){ FAIL(std::string(err)); }
	    
	}

	TEST_CASE("runtime_test_target.SomeClass")
	{
	    std::string function_path = "class=sanity.TestRuntime,callable=getSomeClasses";
	    std::vector<metaffi_type_info> retvals_getSomeClasses_types = {{metaffi_handle_array_type, (char*)"sanity.SomeClass[]", false, 1}};
	
	    xcall* pgetSomeClasses = cppload_entity(g_module_path.string(), function_path, {}, retvals_getSomeClasses_types);
		metaffi::utils::scope_guard sg([pgetSomeClasses]()
		{
			char* err = nullptr;
			free_xcall(pgetSomeClasses, &err);
			if(err){ FAIL(std::string(err)); }
		});
			  
	    cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		cdts_scope_guard(pcdts);
	    cdts& pcdts_params = pcdts[0];
	    cdts& pcdts_retvals = pcdts[1];
	
	    (*pgetSomeClasses)(pcdts, &err);
	    if(err){ FAIL(std::string(err)); }
		
	    REQUIRE((pcdts_retvals[0].type == metaffi_handle_array_type));
	    REQUIRE((pcdts_retvals[0].cdt_val.array_val->fixed_dimensions == 1));
	    REQUIRE((pcdts_retvals[0].cdt_val.array_val->length == 3));
		
		std::vector<cdt_metaffi_handle*> arr = { pcdts_retvals[0].cdt_val.array_val->arr[0].cdt_val.handle_val,
												pcdts_retvals[0].cdt_val.array_val->arr[1].cdt_val.handle_val,
												pcdts_retvals[0].cdt_val.array_val->arr[2].cdt_val.handle_val };
		
	    //--------------------------------------------------------------------
	
	    function_path = "class=sanity.TestRuntime,callable=expectThreeSomeClasses";
	    std::vector<metaffi_type_info> params_expectThreeSomeClasses_types = {{metaffi_handle_array_type, (char*)"sanity.SomeClass[]", false, 1}};
	
	    xcall* pexpectThreeSomeClasses = cppload_entity(g_module_path.string(), function_path, params_expectThreeSomeClasses_types, {});
		metaffi::utils::scope_guard sg2([pexpectThreeSomeClasses]()
		{
			char* err = nullptr;
			free_xcall(pexpectThreeSomeClasses, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
	    cdts* pcdts2 = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		cdts_scope_guard(pcdts2);
	    cdts& pcdts_params2 = pcdts2[0];
	    cdts& pcdts_retvals2 = pcdts2[1];
	
	    pcdts_params2[0].set_new_array(3, 1, metaffi_handle_type);
		pcdts_params2[0].cdt_val.array_val->arr[0].set_handle(arr[0]);
		pcdts_params2[0].cdt_val.array_val->arr[1].set_handle(arr[1]);
		pcdts_params2[0].cdt_val.array_val->arr[2].set_handle(arr[2]);
	
	    (*pexpectThreeSomeClasses)(pcdts2, &err);
	    if(err){ FAIL(std::string(err)); }
	
	    //--------------------------------------------------------------------
	
	    function_path = "class=sanity.SomeClass,callable=print,instance_required";
	    std::vector<metaffi_type_info> params_SomeClassPrint_types = {{metaffi_handle_type, nullptr, false, 0}};
	
	    xcall* pSomeClassPrint = cppload_entity(g_module_path.string(), function_path, params_SomeClassPrint_types, {});
		metaffi::utils::scope_guard sg3([pSomeClassPrint]()
		{
			char* err = nullptr;
			free_xcall(pSomeClassPrint, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
	    cdts* pcdts3 = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		cdts_scope_guard(pcdts3);
	    cdts& pcdts_params3 = pcdts3[0];
	    cdts& pcdts_retvals3 = pcdts3[1];
	
		pcdts_params3[0].set_handle(arr[1]); // use the 2nd instance
	
	    (*pSomeClassPrint)(pcdts3, &err);
	    if(err){ FAIL(std::string(err)); }
	    
	}

	TEST_CASE("runtime_test_target.ThreeBuffers")
	{
	    std::string function_path = "class=sanity.TestRuntime,callable=expectThreeBuffers";
	    std::vector<metaffi_type_info> params_expectThreeBuffers_types = {{metaffi_uint8_array_type, nullptr, false, 2}};
	
	    xcall* pexpectThreeBuffers = cppload_entity(g_module_path.string(), function_path, params_expectThreeBuffers_types, {});
		metaffi::utils::scope_guard sg([pexpectThreeBuffers]()
		{
			char* err = nullptr;
			free_xcall(pexpectThreeBuffers, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
	    cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		cdts_scope_guard(pcdts);
	    cdts& pcdts_params = pcdts[0];
	    cdts& pcdts_retvals = pcdts[1];
	
	    pcdts_params[0].set_new_array(3, 2, metaffi_uint8_array_type);
	    metaffi_uint8 data[3][3] = { {0,1,2}, {3,4,5}, {6,7,8} };
	    pcdts_params[0].cdt_val.array_val->arr[0].set_new_array(3, 1, metaffi_uint8_type);
	    pcdts_params[0].cdt_val.array_val->arr[1].set_new_array(3, 1, metaffi_uint8_type);
	    pcdts_params[0].cdt_val.array_val->arr[2].set_new_array(3, 1, metaffi_uint8_type);
		
		pcdts_params[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = (data[0][0]);
		pcdts_params[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = (data[0][1]);
		pcdts_params[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = (data[0][2]);
		
		pcdts_params[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = (data[1][0]);
		pcdts_params[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = (data[1][1]);
		pcdts_params[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2] = (data[1][2]);
		
		pcdts_params[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0] = (data[2][0]);
		pcdts_params[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[1] = (data[2][1]);
		pcdts_params[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[2] = (data[2][2]);
		
	    (*pexpectThreeBuffers)(pcdts, &err);
	    if(err){ FAIL(std::string(err)); }
	
	    function_path = "class=sanity.TestRuntime,callable=getThreeBuffers";
	    std::vector<metaffi_type_info> retval_getThreeBuffers_types = {{metaffi_uint8_array_type, nullptr, false, 2}};
	
	    xcall* pgetThreeBuffers = cppload_entity(g_module_path.string(), function_path, {}, retval_getThreeBuffers_types);
		metaffi::utils::scope_guard sg2([pgetThreeBuffers]()
		{
			char* err = nullptr;
			free_xcall(pgetThreeBuffers, &err);
			if(err){ FAIL(std::string(err)); }
		});
		
	    cdts* pcdts2 = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		cdts_scope_guard(pcdts2);
	    cdts& pcdts_params2 = pcdts2[0];
		cdts& pcdts_retvals2 = pcdts2[1];
	
	    (*pgetThreeBuffers)(pcdts2, &err);
	    if(err){ FAIL(std::string(err)); }
		
	    REQUIRE((pcdts_retvals2[0].type == metaffi_uint8_array_type));
	    REQUIRE((pcdts_retvals2[0].cdt_val.array_val->fixed_dimensions == 2));
	    REQUIRE((pcdts_retvals2[0].cdt_val.array_val->length == 3));
	    for(int i = 0; i < 3; i++)
		{
	        REQUIRE((pcdts_retvals2[0].cdt_val.array_val->arr[i].cdt_val.array_val->length == 3));
	        for(int j = 0; j < 3; j++)
			{
	            REQUIRE((pcdts_retvals2[0].cdt_val.array_val->arr[i].cdt_val.array_val->arr[j].cdt_val.uint8_val == j+1));
	        }
	    }
	}
	
	TEST_CASE("!return null")
	{
	
	}
	
	TEST_CASE("!return any array")
	{
	
	}
	
	TEST_CASE("!accepts_any")
	{
	
	}
	
	TEST_CASE("call_callback_add")
	{
	
	}
}