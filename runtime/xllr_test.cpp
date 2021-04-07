#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <utils/xllr_api_wrapper.h>
#include <utils/scope_guard.hpp>
#include <boost/filesystem.hpp>
#include <sstream>

using namespace openffi::utils;

#define reset_err \
	out_err = nullptr; \
	out_err_len = 0;

#define check_and_reset_err_success( desc ) \
	if(out_err != nullptr) \
	{ \
		std::string estr(out_err, out_err_len); \
		std::cout << desc << ": " << estr << std::endl; \
		exit(1); \
	}

#define check_and_reset_err_fail( desc ) \
	if(out_err == nullptr) \
	{ \
		std::cout << desc << ": did not fail, although test expected to fail" << std::endl; \
		exit(1); \
	}

#define run_test_step_expect_success( desc, ... ) \
	reset_err; \
	__VA_ARGS__; \
	check_and_reset_err_success(desc);

#define run_test_step_expect_fail( desc, ... ) \
	reset_err; \
	__VA_ARGS__; \
	check_and_reset_err_fail(desc);

//--------------------------------------------------------------------
std::unique_ptr<xllr_api_wrapper> xllr;
std::string plugin_name("xllr.openjdk");
std::string module_name("openjdk_test_mod");
std::string func_name("openjdk_test_func");
std::string class_path(".:");
std::string module_code(
		R"(

import openffi.*;
import java.nio.charset.StandardCharsets;

public class openjdk_test_mod
{
	public openjdk_test_mod(){}

	public static CallResult openjdk_test_func(byte[] arr) throws OpenFFIException
	{
		String s = new String(arr, StandardCharsets.US_ASCII);
		if(!s.equals("input"))
		{
			throw new OpenFFIException("Expected \"input\" in parameters");
		}

		CallResult res = new CallResult();
		res.out_ret = new byte[]{ 114, 101, 115, 117, 108, 116 };

		return res;
	}
}
)"
);

char* out_err = nullptr;
uint32_t out_err_len = 0;

//--------------------------------------------------------------------
void delete_module()
{
	boost::filesystem::remove("openjdk_test_mod.class");
	boost::filesystem::remove("openjdk_test_mod.java");
}
//--------------------------------------------------------------------
void build_module()
{
	// write module code to current dir
	std::ofstream outfile(module_name+".java");
	if(!outfile.is_open()) {
		std::cout << "Couldn't open/create 'openjdk_test_mod.go" << std::endl;
		exit(1);
	}
	
	outfile << module_code << std::endl;
	outfile.close();
	
	std::stringstream ss;
	ss << "javac -cp 'xllr_java_bridge.jar' " << module_name << ".java";
	printf("%s\n", ss.str().c_str());
	// build module
	system(ss.str().c_str());
	
	if(!boost::filesystem::exists("openjdk_test_mod.class"))
	{
		std::cout << "Failed build java test module" << std::endl;
		exit(1);
	}
}
//--------------------------------------------------------------------
void test_module_success()
{
	run_test_step_expect_success("Error in load_runtime_plugin()",
	                             xllr->load_runtime_plugin(plugin_name.c_str(), plugin_name.length(), &out_err, &out_err_len)
	);
	build_module();
	
	// delete module from current dir
	scope_guard sg([&]()
	               {
		               delete_module();
	               });
	
	run_test_step_expect_success("Error in load_module()",
	                             xllr->load_module(plugin_name.c_str(), plugin_name.length(), (class_path+module_name).c_str(), (class_path+module_name).length(), &out_err, &out_err_len)
	);
}
//--------------------------------------------------------------------
void test_module_free_module_via_free_runtime_success()
{
	run_test_step_expect_success("Error in load_runtime_plugin()",
	                             xllr->load_runtime_plugin(plugin_name.c_str(), plugin_name.length(), &out_err, &out_err_len)
	);
	
	build_module();
	
	// delete module from current dir
	scope_guard sg([&]()
	               {
		               delete_module();
	               });
	
	run_test_step_expect_success("Error in load_module()",
	                             xllr->load_module(plugin_name.c_str(), plugin_name.length(), (class_path+module_name).c_str(), (class_path+module_name).length(), &out_err, &out_err_len)
	);
	
}
//--------------------------------------------------------------------
void test_module_lazy_runtime_success()
{
	build_module();
	
	// delete module from current dir
	scope_guard sg([&]()
	               {
		               delete_module();
	               });
	
	// Test 4 - load module + free runtime - lazy loading of runtime
	run_test_step_expect_success("Error in load_module()",
	                             xllr->load_module(plugin_name.c_str(), plugin_name.length(), (class_path+module_name).c_str(), (class_path+module_name).length(), &out_err, &out_err_len)
	);
	
}
//--------------------------------------------------------------------
void test_module_not_exist_fail()
{
	
	run_test_step_expect_fail("Error in load_runtime_plugin()",
	                          xllr->load_module(plugin_name.c_str(), plugin_name.length(), "not_exist", 9, &out_err, &out_err_len)
	);
}
//--------------------------------------------------------------------
void test_call_success()
{
	build_module();
	
	// delete module from current dir
	scope_guard sg([&]()
	               {
		               delete_module();
	               });
	
	std::string params("input");
	unsigned char* out_params = nullptr;
	uint64_t out_params_len = 0;
	unsigned char* out_ret = nullptr;
	uint64_t out_ret_len = 0;
	uint8_t is_error = 0;
	
	// Test 6 - load module that doesn't exist
	xllr->call(plugin_name.c_str(), plugin_name.length(),
	     (class_path+module_name).c_str(), (class_path+module_name).length(),
	     func_name.c_str(), func_name.length(),
	     (unsigned char*)params.c_str(), params.length(),
	     &out_params, &out_params_len,
	     &out_ret, &out_ret_len,
	     &is_error
	);
	
	std::string res(reinterpret_cast<const char*>(out_ret), out_ret_len);
	
	if(res != "result")
	{
		std::cout << "Returned result: '" << res << "' - expected: 'result'" << std::endl;
		exit(1);
	}
}
//--------------------------------------------------------------------
void test_call_fail()
{
	std::string params("input");
	unsigned char* out_params = nullptr;
	uint64_t out_params_len = 0;
	unsigned char* out_ret = nullptr;
	uint64_t out_ret_len = 0;
	uint8_t is_error = 0;
	
	// Test 6 - load module that doesn't exist
	xllr->call(plugin_name.c_str(), plugin_name.length(),
	     (class_path+module_name).c_str(), (class_path+module_name).length(),
	     "dummy", 5,
	     (unsigned char*)params.c_str(), params.length(),
	     &out_params, &out_params_len,
	     &out_ret, &out_ret_len,
	     &is_error
	);
	
	if(!is_error)
	{
		std::cout << "\"call()\" did not fail, although it was expected to as module doesn't exist" << std::endl;
		exit(1);
	}
	
}
//--------------------------------------------------------------------
int main(int argc, char* argv[])
{
	if(argc <= 1)
	{
		std::cout << "Expect test case number. Quitting..." << std::endl;
		return 1;
	}
	
	int testcase = 0;
	
	if(sscanf(argv[1], "%d", &testcase) != 1)
	{
		std::cout << "Given argument is not a test case number. Quitting..." << std::endl;
		return 1;
	}
	
	switch (testcase)
	{
		case 1:
		{
			test_module_success();
		}
			break;
		
		case 2:
		{
			test_module_free_module_via_free_runtime_success();
		}
			break;
		
		case 3:
		{
			test_module_lazy_runtime_success();
		}break;
		
		case 4:
		{
			test_module_not_exist_fail();
		}break;
		
		case 5:
		{
			test_call_success();
		}break;
		
		case 6:
		{
			test_call_fail();
		}break;
		
		default:
			std::cout << "Given test case number does not exist: " << testcase << std::endl;
			return 1;
	}
	
	return 0;
}