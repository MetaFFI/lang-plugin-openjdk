#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include <runtime/runtime_plugin_api.h>
#include <filesystem>
#include <runtime/cdts_wrapper.h>
#include "cdts_java_wrapper.h"
#include "runtime_id.h"

std::filesystem::path g_module_path;
char* err = nullptr;
uint64_t long_err_len = 0;

struct GlobalSetup
{
	GlobalSetup()
	{
		g_module_path = std::filesystem::path(__FILE__);
		g_module_path = g_module_path.parent_path();
		g_module_path.append("test");
		g_module_path.append("TestRuntime.class");
		
		err = nullptr;
		uint32_t err_len_load = 0;
		
		load_runtime(&err, &err_len_load);
		if(err)
		{
			std::cerr << err << std::endl;
			exit(1);
		}
	}
	
	~GlobalSetup()
	{
		err = nullptr;
		uint32_t err_len_free = 0;
		
		free_runtime(&err, &err_len_free);
		if(err)
		{
			std::cerr << err << std::endl;
			exit(2);
		}
	}
};

static GlobalSetup setup;


auto cppload_function(const std::string& module_path,
						const std::string& function_path,
                        std::vector<metaffi_type_info> params_types,
						std::vector<metaffi_type_info> retvals_types)
{
	err = nullptr;
	uint32_t err_len_load = 0;
	
	metaffi_type_info* params_types_arr = params_types.empty() ? nullptr : params_types.data();
	metaffi_type_info* retvals_types_arr = retvals_types.empty() ? nullptr : retvals_types.data();
	
	void** pfunction = load_function(module_path.c_str(), module_path.length(),
									function_path.c_str(), function_path.length(),
									params_types_arr, retvals_types_arr,
									params_types.size(), retvals_types.size(),
									&err, &err_len_load);
	
	if(err)
	{
		FAIL(std::string(err));
	}
	REQUIRE(err_len_load == 0);
	REQUIRE(pfunction[0] != nullptr);
	REQUIRE(pfunction[1] != nullptr);
	
	return pfunction;
};

typedef void(*xcall_no_params_no_retvals)(void*,char**, uint64_t*);

TEST_SUITE("openjdk runtime api")
{
	TEST_CASE("runtime_test_target.hello_world")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=helloWorld";
		void** phello_world = cppload_function(g_module_path.string(), function_path, {}, {});
		
		uint64_t long_err_len = 0;
		((xcall_no_params_no_retvals)phello_world[0])(phello_world[1], &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
	}

	TEST_CASE("runtime_test_target.returns_an_error")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=returnsAnError";
		void** preturns_an_error = cppload_function(g_module_path.string(), function_path, {}, {});

		uint64_t long_err_len = 0;
		((void(*)(void*,char**, uint64_t*))preturns_an_error[0])(preturns_an_error[1], &err, &long_err_len);
		REQUIRE(err != nullptr);
		REQUIRE(long_err_len > 0);
	}

	TEST_CASE("runtime_test_target.div_integers")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=divIntegers";
		std::vector<metaffi_type_info> params_types = { metaffi_type_info{metaffi_int32_type}, metaffi_type_info{metaffi_int32_type}};
		std::vector<metaffi_type_info> retvals_types = { metaffi_type_info{metaffi_float32_type} };

		void** pdiv_integers = cppload_function(g_module_path.string(), function_path, params_types, retvals_types);

		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		pcdts_params[0].type = metaffi_int32_type;
		pcdts_params[0].cdt_val.int32_val = 10;
		pcdts_params[1].type = metaffi_int32_type;
		pcdts_params[1].cdt_val.int32_val = 2;
		
		((void(*)(void*,cdts*,char**,uint64_t*))pdiv_integers[0])(pdiv_integers[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);

		REQUIRE(pcdts_retvals[0].type == metaffi_float32_type);
		REQUIRE(pcdts_retvals[0].cdt_val.float32_val == 5.0);
	}

	TEST_CASE("runtime_test_target.join_strings")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=joinStrings";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_string8_array_type, nullptr, false, 1}};
		std::vector<metaffi_type_info> retvals_types = {metaffi_type_info{metaffi_string8_type}};

		void** join_strings = cppload_function(g_module_path.string(), function_path, params_types, retvals_types);

		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		pcdts_params[0] = cdt(3, 1, metaffi_string8_type);
		pcdts_params[0].cdt_val.array_val[0] = cdt((metaffi_string8)u8"one", false);
		pcdts_params[0].cdt_val.array_val[1] = cdt((metaffi_string8)u8"two", false);
		pcdts_params[0].cdt_val.array_val[2] = cdt((metaffi_string8)u8"three", false);

		((void(*)(void*,cdts*,char**,uint64_t*))join_strings[0])(join_strings[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);

		REQUIRE(pcdts_retvals[0].type == metaffi_string8_type);
		REQUIRE(std::u8string(pcdts_retvals[0].cdt_val.string8_val) == u8"one,two,three");
	}

	TEST_CASE("runtime_test_target.testmap.set_get_contains")
	{
		// create new testmap
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		std::vector<metaffi_type_info> retvals_types = {{metaffi_handle_type, (char*)"sanity/TestMap", false, 0}};

		void** pnew_testmap = cppload_function(g_module_path.string(), function_path, {}, retvals_types);

		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		cdts& params_cdts = pcdts[0];
		cdts& retvals_cdts = pcdts[1];

		((void(*)(void*,cdts*,char**,uint64_t*))pnew_testmap[0])(pnew_testmap[1], (cdts*)pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);

		REQUIRE(retvals_cdts[0].type == metaffi_handle_type);
		REQUIRE(retvals_cdts[0].cdt_val.handle_val.val != nullptr);
		REQUIRE(retvals_cdts[0].cdt_val.handle_val.runtime_id == OPENJDK_RUNTIME_ID);

		cdt_metaffi_handle testmap_instance = retvals_cdts[0].cdt_val.handle_val;

		// set
		function_path = "class=sanity.TestMap,callable=set,instance_required";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}, metaffi_type_info{metaffi_any_type}};

		void** p_testmap_set = cppload_function(g_module_path.string(), function_path, params_types, {});

		pcdts = (cdts*)xllr_alloc_cdts_buffer(3, 0);
		params_cdts = std::move(pcdts[0]);
		retvals_cdts = std::move(pcdts[1]);
		
		params_cdts[0] = cdt(testmap_instance);
		params_cdts[1] = cdt((metaffi_string8)std::u8string(u8"key").c_str(), true);
		params_cdts[2] = cdt((int32_t)42);

		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_set[0])(p_testmap_set[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		// contains
		function_path = "class=sanity.TestMap,callable=contains,instance_required";
		std::vector<metaffi_type_info> params_contains_types = { metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type} };
		std::vector<metaffi_type_info> retvals_contains_types = { metaffi_type_info{metaffi_bool_type} };

		void** p_testmap_contains = cppload_function(g_module_path.string(), function_path, params_contains_types, retvals_contains_types);

		pcdts = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		params_cdts = std::move(pcdts[0]);
		retvals_cdts = std::move(pcdts[1]);
		
		params_cdts[0] = cdt(testmap_instance);
		params_cdts[1] = cdt((metaffi_string8)u8"key", true);

		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_contains[0])(p_testmap_contains[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);

		REQUIRE(retvals_cdts[0].type == metaffi_bool_type);
		REQUIRE(retvals_cdts[0].cdt_val.bool_val != 0);
		
		// get
		function_path = "class=sanity.TestMap,callable=get,instance_required";
		std::vector<metaffi_type_info> params_get_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};
		std::vector<metaffi_type_info> retvals_get_types = {metaffi_type_info{metaffi_any_type}};

		void** p_testmap_get = cppload_function(g_module_path.string(), function_path, params_get_types, retvals_get_types);

		pcdts = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		params_cdts = std::move(pcdts[0]);
		retvals_cdts = std::move(pcdts[1]);
		
		params_cdts[0] = cdt(testmap_instance);
		params_cdts[1] = cdt((char8_t*)u8"key", true);

 		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_get[0])(p_testmap_get[1], (cdts*)pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);

		REQUIRE(retvals_cdts[0].type == metaffi_int32_type);
		REQUIRE(retvals_cdts[0].cdt_val.int32_val == 42);
	}

	TEST_CASE("runtime_test_target.testmap.set_get_contains_cpp_object")
	{
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		std::vector<metaffi_type_info> retvals_types = {{metaffi_handle_type, (char*)"sanity/TestMap", false, 0}};
		
		void** pnew_testmap = cppload_function(g_module_path.string(), function_path, {}, retvals_types);
		
		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		
		((void(*)(void*,cdts*,char**,uint64_t*))pnew_testmap[0])(pnew_testmap[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		REQUIRE(pcdts_retvals[0].type == metaffi_handle_type);
		REQUIRE(pcdts_retvals[0].cdt_val.handle_val.val != nullptr);
		REQUIRE(pcdts_retvals[0].cdt_val.handle_val.runtime_id == OPENJDK_RUNTIME_ID);
		
		cdt_metaffi_handle testmap_instance = pcdts_retvals[0].cdt_val.handle_val;
		
		// set
		function_path = "class=sanity.TestMap,callable=set,instance_required";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}, metaffi_type_info{metaffi_any_type}};
		
		void** p_testmap_set = cppload_function(g_module_path.string(), function_path, params_types, {});
		
		pcdts = (cdts*)xllr_alloc_cdts_buffer(3, 0);
		pcdts_params = std::move(pcdts[0]);
		pcdts_retvals = std::move(pcdts[1]);
		
		std::vector<int> vec_to_insert = {1,2,3};
		
		pcdts_params[0] = cdt(testmap_instance);
		pcdts_params[1] = cdt((metaffi_string8)u8"key", true);
		pcdts_params[2] = cdt(cdt_metaffi_handle{&vec_to_insert, 733, nullptr});

		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_set[0])(p_testmap_set[1], (cdts*)pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);

		// contains
		function_path = "class=sanity.TestMap,callable=contains,instance_required";
		std::vector<metaffi_type_info> params_contains_types = { metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type} };
		std::vector<metaffi_type_info> retvals_contains_types = { metaffi_type_info{metaffi_bool_type} };
		
		void** p_testmap_contains = cppload_function(g_module_path.string(), function_path, params_contains_types, retvals_contains_types);
		
		pcdts = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		pcdts_params = std::move(pcdts[0]);
		pcdts_retvals = std::move(pcdts[1]);
		
		pcdts_params[0] = cdt(testmap_instance);
		pcdts_params[1] = cdt((metaffi_string8)u8"key", true);
		
		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_contains[0])(p_testmap_contains[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		REQUIRE(pcdts_retvals[0].type == metaffi_bool_type);
		REQUIRE(pcdts_retvals[0].cdt_val.bool_val != 0);
		
		
		// get
		function_path = "class=sanity.TestMap,callable=get,instance_required";
		std::vector<metaffi_type_info> params_get_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};
		std::vector<metaffi_type_info> retvals_get_types = {metaffi_type_info{metaffi_any_type}};
		
		void** p_testmap_get = cppload_function(g_module_path.string(), function_path, params_get_types, retvals_get_types);
		
		pcdts = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		pcdts_params = std::move(pcdts[0]);
		pcdts_retvals = std::move(pcdts[1]);
		
		pcdts_params[0] = cdt(testmap_instance);
		pcdts_params[1] = cdt((char8_t*)u8"key", true);
		
		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_get[0])(p_testmap_get[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		REQUIRE(pcdts_retvals[0].type == metaffi_handle_type);
		auto& vector_pulled = *(std::vector<int>*)pcdts_retvals[0].cdt_val.handle_val.val;

		REQUIRE(vector_pulled[0] == 1);
		REQUIRE(vector_pulled[1] == 2);
		REQUIRE(vector_pulled[2] == 3);

	}

	TEST_CASE("runtime_test_target.testmap.get_set_name")
	{
		// create new testmap
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		std::vector<metaffi_type_info> retvals_types = {{metaffi_handle_type, (char*)"sanity/TestMap", false, 0}};
		
		void** pnew_testmap = cppload_function(g_module_path.string(), function_path, {}, retvals_types);
		
		cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		cdts& pcdts_params = pcdts[0];
		cdts& pcdts_retvals = pcdts[1];
		
		((void(*)(void*,cdts*,char**,uint64_t*))pnew_testmap[0])(pnew_testmap[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		REQUIRE(pcdts_retvals[0].type == metaffi_handle_type);
		REQUIRE(pcdts_retvals[0].cdt_val.handle_val.val != nullptr);
		REQUIRE(pcdts_retvals[0].cdt_val.handle_val.runtime_id == OPENJDK_RUNTIME_ID);
		
		cdt_metaffi_handle testmap_instance = pcdts_retvals[0].cdt_val.handle_val;
		
		
		// load getter
		function_path = "class=sanity.TestMap,field=name,instance_required,getter";
		std::vector<metaffi_type_info> params_name_getter_types = {metaffi_type_info{metaffi_handle_type}};
		std::vector<metaffi_type_info> retvals_name_getter_types = {metaffi_type_info{metaffi_string8_type}};
		
		void** pget_name = cppload_function(g_module_path.string(), function_path, params_name_getter_types, retvals_name_getter_types);
		
		// load setter
		function_path = "class=sanity.TestMap,field=name,instance_required,setter";
		std::vector<metaffi_type_info> params_name_setter_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};
		
		void** pset_name = cppload_function(g_module_path.string(), function_path, params_name_setter_types, {});
		
		// get name
		pcdts = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		pcdts_params = std::move(pcdts[0]);
		pcdts_retvals = std::move(pcdts[1]);
		
		pcdts_params[0] = cdt(testmap_instance);
		
		((void(*)(void*,cdts*,char**,uint64_t*))pget_name[0])(pget_name[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		REQUIRE(pcdts_retvals[0].type == metaffi_string8_type);
		REQUIRE(std::u8string(pcdts_retvals[0].cdt_val.string8_val).empty());
		
		// set name to "name is my name"
		pcdts = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		pcdts_params = std::move(pcdts[0]);
		pcdts_retvals = std::move(pcdts[1]);
		
		pcdts_params[0] = cdt(testmap_instance);
		pcdts_params[1] = cdt((metaffi_string8)u8"name is my name", true);
		
		((void(*)(void*,cdts*,char**,uint64_t*))pset_name[0])(pset_name[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		// get name again and make sure it is "name is my name"
		pcdts = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		pcdts_params = std::move(pcdts[0]);
		pcdts_retvals = std::move(pcdts[1]);
		
		pcdts_params[0] = cdt(testmap_instance);
		
		((void(*)(void*,cdts*,char**,uint64_t*))pget_name[0])(pget_name[1], pcdts, &err, &long_err_len);
		if(err){ FAIL(std::string(err)); }
		REQUIRE(long_err_len == 0);
		
		REQUIRE(pcdts_retvals[0].type == metaffi_string8_type);
		REQUIRE(std::u8string(pcdts_retvals[0].cdt_val.string8_val) == u8"name is my name");
	}

	TEST_CASE("runtime_test_target.wait_a_bit")
	{
	    // get five_seconds global
	    std::string function_path = "class=sanity.TestRuntime,field=fiveSeconds,getter";
	    std::vector<metaffi_type_info> retvals_fiveSeconds_getter_types = {{metaffi_int32_type, nullptr, false, 0}};
	
	    void** pfive_seconds_getter = cppload_function(g_module_path.string(), function_path, {}, retvals_fiveSeconds_getter_types);
	
	    cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
	    cdts& pcdts_params = pcdts[0];
	    cdts& pcdts_retvals = pcdts[1];
	
	    ((void(*)(void*,cdts*,char**,uint64_t*))pfive_seconds_getter[0])(pfive_seconds_getter[1], pcdts, &err, &long_err_len);
	    if(err){ FAIL(std::string(err)); }
	    REQUIRE(long_err_len == 0);
	
	    REQUIRE(pcdts_retvals[0].type == metaffi_int32_type);
	    REQUIRE(pcdts_retvals[0].cdt_val.int32_val == 5);
	
	    int32_t five = pcdts_retvals[0].cdt_val.int32_val;
	
	    // call wait_a_bit
	    function_path = "class=sanity.TestRuntime,callable=waitABit";
	    std::vector<metaffi_type_info> params_waitABit_types = {metaffi_type_info{metaffi_int32_type}};
	
	    void** pwait_a_bit = cppload_function(g_module_path.string(), function_path, params_waitABit_types, {});
	
	    pcdts = (cdts*)xllr_alloc_cdts_buffer(1, 0);
	    pcdts_params = std::move(pcdts[0]);
	    pcdts_retvals = std::move(pcdts[1]);
	
	    pcdts_params[0] = cdt(five);
	
	    ((void(*)(void*,cdts*,char**,uint64_t*))pwait_a_bit[0])(pwait_a_bit[1], pcdts, &err, &long_err_len);
	    if(err){ FAIL(std::string(err)); }
	    REQUIRE(long_err_len == 0);
	}

	TEST_CASE("runtime_test_target.SomeClass")
	{
	    std::string function_path = "class=sanity.TestRuntime,callable=getSomeClasses";
	    std::vector<metaffi_type_info> retvals_getSomeClasses_types = {{metaffi_handle_array_type, (char*)"sanity.SomeClass[]", false, 1}};
	
	    void** pgetSomeClasses = cppload_function(g_module_path.string(), function_path, {}, retvals_getSomeClasses_types);
	
	    cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
	    cdts& pcdts_params = pcdts[0];
	    cdts& pcdts_retvals = pcdts[1];
	
	    ((void(*)(void*,cdts*,char**,uint64_t*))pgetSomeClasses[0])(pgetSomeClasses[1], pcdts, &err, &long_err_len);
	    if(err){ FAIL(std::string(err)); }
	    REQUIRE(long_err_len == 0);
	
	    REQUIRE(pcdts_retvals[0].type == metaffi_handle_array_type);
	    REQUIRE(pcdts_retvals[0].cdt_val.array_val.fixed_dimensions == 1);
	    REQUIRE(pcdts_retvals[0].cdt_val.array_val.length == 3);
		
		std::vector<cdt_metaffi_handle> arr = { pcdts_retvals[0].cdt_val.array_val.arr[0].cdt_val.handle_val,
												pcdts_retvals[0].cdt_val.array_val.arr[1].cdt_val.handle_val,
												pcdts_retvals[0].cdt_val.array_val.arr[2].cdt_val.handle_val };
		
	    //--------------------------------------------------------------------
	
	    function_path = "class=sanity.TestRuntime,callable=expectThreeSomeClasses";
	    std::vector<metaffi_type_info> params_expectThreeSomeClasses_types = {{metaffi_handle_array_type, (char*)"sanity.SomeClass[]", false, 1}};
	
	    void** pexpectThreeSomeClasses = cppload_function(g_module_path.string(), function_path, params_expectThreeSomeClasses_types, {});
	
	    pcdts = (cdts*)xllr_alloc_cdts_buffer(1, 0);
	    pcdts_params = std::move(pcdts[0]);
	    pcdts_retvals = std::move(pcdts[1]);
	
	    pcdts_params[0] = cdt(3, 1, metaffi_handle_type);
		pcdts_params[0].cdt_val.array_val[0] = cdt(arr[0]);
		pcdts_params[0].cdt_val.array_val[1] = cdt(arr[1]);
		pcdts_params[0].cdt_val.array_val[2] = cdt(arr[2]);
	
	    ((void(*)(void*,cdts*,char**,uint64_t*))pexpectThreeSomeClasses[0])(pexpectThreeSomeClasses[1], pcdts, &err, &long_err_len);
	    if(err){ FAIL(std::string(err)); }
	    REQUIRE(long_err_len == 0);
	
	    //--------------------------------------------------------------------
	
	    function_path = "class=sanity.SomeClass,callable=print,instance_required";
	    std::vector<metaffi_type_info> params_SomeClassPrint_types = {{metaffi_handle_type, nullptr, false, 0}};
	
	    void** pSomeClassPrint = cppload_function(g_module_path.string(), function_path, params_SomeClassPrint_types, {});
	
	    pcdts = (cdts*)xllr_alloc_cdts_buffer(1, 0);
	    pcdts_params = std::move(pcdts[0]);
	    pcdts_retvals = std::move(pcdts[1]);
	
	    pcdts_params[0] = cdt(arr[1]); // use the 2nd instance
	
	    ((void(*)(void*,cdts*,char**,uint64_t*))pSomeClassPrint[0])(pSomeClassPrint[1], pcdts, &err, &long_err_len);
	    if(err){ FAIL(std::string(err)); }
	    REQUIRE(long_err_len == 0);
	}

	TEST_CASE("runtime_test_target.ThreeBuffers")
	{
	    std::string function_path = "class=sanity.TestRuntime,callable=expectThreeBuffers";
	    std::vector<metaffi_type_info> params_expectThreeBuffers_types = {{metaffi_uint8_array_type, nullptr, false, 2}};
	
	    void** pexpectThreeBuffers = cppload_function(g_module_path.string(), function_path, params_expectThreeBuffers_types, {});
	
	    cdts* pcdts = (cdts*)xllr_alloc_cdts_buffer(1, 0);
	    cdts& pcdts_params = pcdts[0];
	    cdts& pcdts_retvals = pcdts[1];
	
	    pcdts_params[0] = cdt(3, 2, metaffi_uint8_array_type);
	    metaffi_uint8 data[3][3] = { {0,1,2}, {3,4,5}, {6,7,8} };
	    pcdts_params[0].cdt_val.array_val[0] = cdt(3, 1, metaffi_uint8_type);
	    pcdts_params[0].cdt_val.array_val[1] = cdt(3, 1, metaffi_uint8_type);
	    pcdts_params[0].cdt_val.array_val[2] = cdt(3, 1, metaffi_uint8_type);
		
		pcdts_params[0].cdt_val.array_val[0].cdt_val.array_val[0] = cdt(data[0][0]);
		pcdts_params[0].cdt_val.array_val[0].cdt_val.array_val[1] = cdt(data[0][1]);
		pcdts_params[0].cdt_val.array_val[0].cdt_val.array_val[2] = cdt(data[0][2]);
		
		pcdts_params[0].cdt_val.array_val[1].cdt_val.array_val[0] = cdt(data[1][0]);
		pcdts_params[0].cdt_val.array_val[1].cdt_val.array_val[1] = cdt(data[1][1]);
		pcdts_params[0].cdt_val.array_val[1].cdt_val.array_val[2] = cdt(data[1][2]);
		
		pcdts_params[0].cdt_val.array_val[2].cdt_val.array_val[0] = cdt(data[2][0]);
		pcdts_params[0].cdt_val.array_val[2].cdt_val.array_val[1] = cdt(data[2][1]);
		pcdts_params[0].cdt_val.array_val[2].cdt_val.array_val[2] = cdt(data[2][2]);
		
	    ((void(*)(void*,cdts*,char**,uint64_t*))pexpectThreeBuffers[0])(pexpectThreeBuffers[1], pcdts, &err, &long_err_len);
	    if(err){ FAIL(std::string(err)); }
	    REQUIRE(long_err_len == 0);
	
	    function_path = "class=sanity.TestRuntime,callable=getThreeBuffers";
	    std::vector<metaffi_type_info> retval_getThreeBuffers_types = {{metaffi_uint8_array_type, nullptr, false, 2}};
	
	    void** pgetThreeBuffers = cppload_function(g_module_path.string(), function_path, {}, retval_getThreeBuffers_types);
	
	    pcdts = (cdts*)xllr_alloc_cdts_buffer(0, 1);
	    pcdts_params = std::move(pcdts[0]);
	    pcdts_retvals = std::move(pcdts[1]);
	
	    ((void(*)(void*,cdts*,char**,uint64_t*))pgetThreeBuffers[0])(pgetThreeBuffers[1], pcdts, &err, &long_err_len);
	    if(err){ FAIL(std::string(err)); }
	    REQUIRE(long_err_len == 0);
	
	    REQUIRE(pcdts_retvals[0].type == metaffi_uint8_array_type);
	    REQUIRE(pcdts_retvals[0].cdt_val.array_val.fixed_dimensions == 2);
	    REQUIRE(pcdts_retvals[0].cdt_val.array_val.length == 3);
	    for(int i = 0; i < 3; i++) {
	        REQUIRE(pcdts_retvals[0].cdt_val.array_val.arr[i].cdt_val.array_val.length == 3);
	        for(int j = 0; j < 3; j++) {
	            REQUIRE(pcdts_retvals[0].cdt_val.array_val.arr[i].cdt_val.array_val[j].cdt_val.uint8_val == j+1);
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