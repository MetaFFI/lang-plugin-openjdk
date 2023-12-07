#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_NO_WINDOWS_SEH

#include <catch2/catch.hpp>
#include <runtime/runtime_plugin_api.h>
#include <filesystem>
#include <runtime/cdts_wrapper.h>
#include "cdts_java_wrapper.h"

// Next time use CATCH_GLOBAL_FIXTURE for setup

TEST_CASE( "openjdk runtime api", "[openjdkruntime]" )
{
	REQUIRE(std::getenv("METAFFI_HOME") != nullptr);
	
	std::filesystem::path module_path(__FILE__);
	module_path = module_path.parent_path();
	module_path.append("test");
	module_path.append("TestRuntime.class");
	char* err = nullptr;
	uint32_t err_len = 0;
	
	SECTION("Load Runtime")
	{
		load_runtime(&err, &err_len);
		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
	}
	
	SECTION("runtime_test_target.hello_world")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=helloWorld";
		void** phello_world = load_function(module_path.string().c_str(), module_path.string().length(),
											function_path.c_str(), function_path.length(),
											nullptr, nullptr,
											  0, 0,
											  &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(phello_world[0] != nullptr);
		REQUIRE(phello_world[1] != nullptr);

		uint64_t long_err_len = 0;
		((void(*)(void*,char**, uint64_t*))phello_world[0])(phello_world[1], &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);
	}
	
	SECTION("runtime_test_target.returns_an_error")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=returnsAnError";
		void** preturns_an_error = load_function(module_path.string().c_str(), module_path.string().length(),
		                                         function_path.c_str(), function_path.length(),
		                                         nullptr, nullptr,
		                                         0, 0,
		                                         &err, &err_len);
		
		REQUIRE(err_len == 0);
		REQUIRE(preturns_an_error[0] != nullptr);
		REQUIRE(preturns_an_error[1] != nullptr);
		
		uint64_t long_err_len = 0;
		((void(*)(void*,char**, uint64_t*))preturns_an_error[0])(preturns_an_error[1], &err, &long_err_len);
		REQUIRE(err != nullptr);
		REQUIRE(long_err_len > 0);
		
		SUCCEED(std::string(err, long_err_len).c_str());
	}
	
	SECTION("runtime_test_target.div_integers")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=divIntegers";
		metaffi_type_with_alias params_types[] = {{metaffi_int32_type}, {metaffi_int32_type}};
		metaffi_type_with_alias retvals_types[] = {{metaffi_float32_type}};
		
		uint8_t params_count = 2;
		uint8_t retvals_count = 1;
		
		void** pdiv_integers = load_function(module_path.string().c_str(), module_path.string().length(),
		                                     function_path.c_str(), function_path.length(),
		                                     params_types, retvals_types,
		                                     params_count, retvals_count,
		                                     &err, &err_len);
		
		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(pdiv_integers[0] != nullptr);
		REQUIRE(pdiv_integers[1] != nullptr);
		
		
		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(params_count, retvals_count);
		metaffi::runtime::cdts_wrapper wrapper(cdts_param_ret[0].pcdt, cdts_param_ret[0].len, false);
		wrapper[0]->type = metaffi_int64_type;
		wrapper[0]->cdt_val.metaffi_int64_val.val = 10;
		wrapper[1]->type = metaffi_int64_type;
		wrapper[1]->cdt_val.metaffi_int64_val.val = 2;
		
		uint64_t long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pdiv_integers[0])(pdiv_integers[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);
		
		metaffi::runtime::cdts_wrapper wrapper_ret(cdts_param_ret[1].pcdt, cdts_param_ret[1].len, false);
		REQUIRE(wrapper_ret[0]->type == metaffi_float32_type);
		REQUIRE(wrapper_ret[0]->cdt_val.metaffi_float32_val.val == 5.0);
		
		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}
	}
		
	SECTION("runtime_test_target.join_strings")
	{
		std::string function_path = "class=sanity.TestRuntime,callable=joinStrings";
		metaffi_type_with_alias params_types[] = {{metaffi_string8_array_type}};
		metaffi_type_with_alias retvals_types[] = {{metaffi_string8_type}};

		void** join_strings = load_function(module_path.string().c_str(), module_path.string().length(),
		                                     function_path.c_str(), function_path.length(),
		                                     params_types, retvals_types,
		                                     1, 1,
		                                     &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(join_strings[0] != nullptr);
		REQUIRE(join_strings[1] != nullptr);


		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		metaffi::runtime::cdts_wrapper wrapper(cdts_param_ret[0].pcdt, cdts_param_ret[0].len, false);
		metaffi_size array_dimensions;
		metaffi_size array_length[] = {3};
		metaffi_string8 values[] = {(metaffi_string8)"one", (metaffi_string8)"two", (metaffi_string8)"three"};
		metaffi_size vals_length[] = {strlen("one"), strlen("two"), strlen("three")};
		wrapper[0]->type = metaffi_string8_array_type;
		wrapper[0]->cdt_val.metaffi_string8_array_val.dimensions = 1;
		wrapper[0]->cdt_val.metaffi_string8_array_val.dimensions_lengths = (metaffi_size*)array_length;
		wrapper[0]->cdt_val.metaffi_string8_array_val.vals = values;
		wrapper[0]->cdt_val.metaffi_string8_array_val.vals_sizes = vals_length;

		uint64_t long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))join_strings[0])(join_strings[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		metaffi::runtime::cdts_wrapper wrapper_ret(cdts_param_ret[1].pcdt, cdts_param_ret[1].len, false);
		REQUIRE(wrapper_ret[0]->type == metaffi_string8_type);

		std::string returned(wrapper_ret[0]->cdt_val.metaffi_string8_val.val, wrapper_ret[0]->cdt_val.metaffi_string8_val.length);
		REQUIRE(returned == "one,two,three");

		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}
	}
	
	SECTION("runtime_test_target.testmap.set_get_contains")
	{
		// create new testmap
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		metaffi_type_with_alias retvals_types[] = {{metaffi_handle_type, (char*)"sanity/TestMap", strlen("sanity/TestMap")}};
		
		void** pnew_testmap = load_function(module_path.string().c_str(), module_path.string().length(),
		                                    function_path.c_str(), function_path.length(),
		                                    nullptr, retvals_types,
		                                    0, 1,
		                                    &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(pnew_testmap[0] != nullptr);
		REQUIRE(pnew_testmap[1] != nullptr);

		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);

		uint64_t long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pnew_testmap[0])(pnew_testmap[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		metaffi::runtime::cdts_wrapper wrapper_ret(cdts_param_ret[1].pcdt, cdts_param_ret[1].len, false);
		REQUIRE(wrapper_ret[0]->type == metaffi_handle_type);
		REQUIRE(wrapper_ret[0]->cdt_val.metaffi_handle_val.val != nullptr);

		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}

		metaffi_handle testmap_instance = wrapper_ret[0]->cdt_val.metaffi_handle_val.val;

		// set
		function_path = "class=sanity.TestMap,callable=set,instance_required";
		metaffi_type_with_alias params_types[] = {{metaffi_handle_type}, {metaffi_string8_type}, {metaffi_any_type}};

		void** p_testmap_set = load_function(module_path.string().c_str(), module_path.string().length(),
		                                    function_path.c_str(), function_path.length(),
		                                     params_types, nullptr,
		                                    3, 0,
		                                    &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(p_testmap_set[0] != nullptr);
		REQUIRE(p_testmap_set[1] != nullptr);

		cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(3, 0);
		cdts_java_wrapper wrapper(cdts_param_ret[0].pcdt, cdts_param_ret[0].len);
		wrapper.set(0, testmap_instance);
		wrapper.set(1, std::string("key"));
		wrapper.set(2, (int32_t)42);

		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_set[0])(p_testmap_set[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}

		// contains
		function_path = "class=sanity.TestMap,callable=contains,instance_required";
		metaffi_type_with_alias params_contains_types[] = {{metaffi_handle_type}, {metaffi_string8_type}};
		metaffi_type_with_alias retvals_contains_types[] = {{metaffi_bool_type}};

		void** p_testmap_contains = load_function(module_path.string().c_str(), module_path.string().length(),
		                                     function_path.c_str(), function_path.length(),
                                             params_contains_types, retvals_contains_types,
		                                     2, 1,
		                                     &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(p_testmap_contains[0] != nullptr);
		REQUIRE(p_testmap_contains[1] != nullptr);

		cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		cdts_java_wrapper wrapper_contains_params(cdts_param_ret[0].pcdt, cdts_param_ret[0].len);
		wrapper_contains_params.set(0, testmap_instance);
		wrapper_contains_params.set(1, std::string("key"));

		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_contains[0])(p_testmap_contains[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		metaffi::runtime::cdts_wrapper wrapper_contains_ret(cdts_param_ret[1].pcdt, cdts_param_ret[1].len, false);
		REQUIRE(wrapper_contains_ret[0]->type == metaffi_bool_type);
		REQUIRE(wrapper_contains_ret[0]->cdt_val.metaffi_bool_val.val != 0);

		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}

		// get
		function_path = "class=sanity.TestMap,callable=get,instance_required";
		metaffi_type_with_alias params_get_types[] = {{metaffi_handle_type}, {metaffi_string8_type}};
		metaffi_type_with_alias retvals_get_types[] = {{metaffi_any_type}};

		void** p_testmap_get = load_function(module_path.string().c_str(), module_path.string().length(),
		                                          function_path.c_str(), function_path.length(),
		                                     params_get_types, retvals_get_types,
		                                          2, 1,
		                                          &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(p_testmap_get[0] != nullptr);
		REQUIRE(p_testmap_get[1] != nullptr);

		cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		metaffi::runtime::cdts_wrapper wrapper_get_params(cdts_param_ret[0].pcdt, cdts_param_ret[0].len, false);
		wrapper_get_params[0]->type = metaffi_handle_type;
		wrapper_get_params[0]->cdt_val.metaffi_handle_val.val = testmap_instance;
		wrapper_get_params[1]->type = metaffi_string8_type;
		wrapper_get_params[1]->cdt_val.metaffi_string8_val.val = (char*)"key";
		wrapper_get_params[1]->cdt_val.metaffi_string8_val.length = strlen("key");

		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))p_testmap_get[0])(p_testmap_get[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		metaffi::runtime::cdts_wrapper wrapper_get_ret(cdts_param_ret[1].pcdt, cdts_param_ret[1].len, false);
		REQUIRE(wrapper_get_ret[0]->type == metaffi_int32_type);
		REQUIRE(wrapper_get_ret[0]->cdt_val.metaffi_int32_val.val == 42);

		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}
	}
	
	SECTION("runtime_test_target.testmap.get_set_name")
	{
		// create new testmap
		std::string function_path = "class=sanity.TestMap,callable=<init>";
		metaffi_type_with_alias retvals_types[] = {{metaffi_handle_type, (char*)"sanity/TestMap", strlen("sanity/TestMap")}};
		
		void** pnew_testmap = load_function(module_path.string().c_str(), module_path.string().length(),
		                                    function_path.c_str(), function_path.length(),
		                                    nullptr, retvals_types,
		                                    0, 1,
		                                    &err, &err_len);
		
		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(pnew_testmap[0] != nullptr);
		REQUIRE(pnew_testmap[1] != nullptr);
		
		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		
		uint64_t long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pnew_testmap[0])(pnew_testmap[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);
		
		metaffi::runtime::cdts_wrapper wrapper_ret(cdts_param_ret[1].pcdt, cdts_param_ret[1].len, false);
		REQUIRE(wrapper_ret[0]->type == metaffi_handle_type);
		REQUIRE(wrapper_ret[0]->cdt_val.metaffi_handle_val.val != nullptr);
		
		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}
		
		metaffi_handle testmap_instance = wrapper_ret[0]->cdt_val.metaffi_handle_val.val;
		
		
		// load getter
		function_path = "class=sanity.TestMap,field=name,instance_required,getter";
		metaffi_type_with_alias params_name_getter_types[] = {{metaffi_handle_type}};
		metaffi_type_with_alias retvals_name_getter_types[] = {{metaffi_string8_type}};
		
		
		void** pget_name = load_function(module_path.string().c_str(), module_path.string().length(),
		                                    function_path.c_str(), function_path.length(),
		                                    params_name_getter_types, retvals_name_getter_types,
		                                    1, 1,
		                                    &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(pget_name[0] != nullptr);
		REQUIRE(pget_name[1] != nullptr);

		// load setter
		function_path = "class=sanity.TestMap,field=name,instance_required,setter";
		metaffi_type_with_alias params_name_setter_types[] = {{metaffi_handle_type}, {metaffi_string8_type}};

		void** pset_name = load_function(module_path.string().c_str(), module_path.string().length(),
		                                 function_path.c_str(), function_path.length(),
		                                 params_name_setter_types, nullptr,
		                                 2, 0,
		                                 &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(pset_name[0] != nullptr);
		REQUIRE(pset_name[1] != nullptr);
		

		// get name
		cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		cdts_java_wrapper wrapper_name_getter_params(cdts_param_ret[0].pcdt, cdts_param_ret[0].len);
		wrapper_name_getter_params.set(0, testmap_instance);

		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pget_name[0])(pget_name[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		metaffi::runtime::cdts_wrapper wrapper_get_ret(cdts_param_ret[1].pcdt, cdts_param_ret[1].len, false);
		REQUIRE(wrapper_get_ret[0]->type == metaffi_string8_type);
		REQUIRE(std::string(wrapper_get_ret[0]->cdt_val.metaffi_string8_val.val, wrapper_get_ret[0]->cdt_val.metaffi_string8_val.length).empty());

		// set name to "name is my name"
		cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		cdts_java_wrapper wrapper_set_params(cdts_param_ret[0].pcdt, cdts_param_ret[0].len);
		wrapper_set_params.set(0, testmap_instance);
		wrapper_set_params.set(1, std::string("name is my name"));
		
		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pset_name[0])(pset_name[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		// get name again and make sure it is "name is my name"
		cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		cdts_java_wrapper wrapper_last_name_getter_params(cdts_param_ret[0].pcdt, cdts_param_ret[0].len);
		wrapper_name_getter_params.set(0, testmap_instance);
		
		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pget_name[0])(pget_name[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		cdts_java_wrapper last_get_wrapper(cdts_param_ret[1].pcdt, cdts_param_ret[1].len);
		REQUIRE(wrapper_get_ret[0]->type == metaffi_string8_type);
		REQUIRE(std::string(wrapper_get_ret[0]->cdt_val.metaffi_string8_val.val, last_get_wrapper[0]->cdt_val.metaffi_string8_val.length) == "name is my name");
	}
	
	SECTION("runtime_test_target.wait_a_bit")
	{
		// get five_seconds global
		std::string function_path = "class=sanity.TestRuntime,field=fiveSeconds,getter";
		metaffi_type_with_alias retvals_fiveSeconds_getter_types[] = {{metaffi_int32_type}};
		
		
		void** pfive_seconds_getter = load_function(module_path.string().c_str(), module_path.string().length(),
		                                 function_path.c_str(), function_path.length(),
		                                 nullptr, retvals_fiveSeconds_getter_types,
		                                 0, 1,
		                                 &err, &err_len);
		
		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(pfive_seconds_getter[0] != nullptr);
		REQUIRE(pfive_seconds_getter[1] != nullptr);
		
		cdts* getter_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);

		uint64_t long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pfive_seconds_getter[0])(pfive_seconds_getter[1], (cdts*)getter_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		
		cdts_java_wrapper last_get_wrapper(getter_ret[1].pcdt, getter_ret[1].len);
		REQUIRE(last_get_wrapper[0]->type == metaffi_int32_type);
		REQUIRE(last_get_wrapper[0]->cdt_val.metaffi_int32_val.val == 5);
		
		int32_t five = last_get_wrapper[0]->cdt_val.metaffi_int32_val.val;

		// call wait_a_bit
		function_path = "class=sanity.TestRuntime,callable=waitABit";
		metaffi_type_with_alias params_waitABit_types[] = {{metaffi_int32_type}};

		void** pwait_a_bit = load_function(module_path.string().c_str(), module_path.string().length(),
		                                   function_path.c_str(), function_path.length(),
		                                   params_waitABit_types, nullptr,
		                                   1, 0,
		                                   &err, &err_len);

		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
		REQUIRE(pwait_a_bit[0] != nullptr);
		REQUIRE(pwait_a_bit[1] != nullptr);


		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		cdts_java_wrapper wrapper(cdts_param_ret[0].pcdt, cdts_param_ret[0].len);
		wrapper.set(0, five);

		long_err_len = 0;
		((void(*)(void*,cdts*,char**,uint64_t*))pwait_a_bit[0])(pwait_a_bit[1], (cdts*)cdts_param_ret, &err, &long_err_len);
		if(err){ FAIL(err); }
		REQUIRE(long_err_len == 0);

		if(cdts_param_ret[0].len + cdts_param_ret[1].len > cdts_cache_size){
			free(cdts_param_ret);
		}
	}

	
	SECTION("Free Runtime")
	{
		free_runtime(&err, &err_len);
		if(err){ FAIL(err); }
		REQUIRE(err_len == 0);
	}

}