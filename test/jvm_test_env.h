#pragma once

#include <metaffi/api/metaffi_api.h>

#include <string>

struct JvmTestEnv
{
	metaffi::api::MetaFFIRuntime runtime;
	std::string guest_classpath;
	metaffi::api::MetaFFIModule guest_module;

	JvmTestEnv();
	~JvmTestEnv();
};

JvmTestEnv& jvm_test_env();

std::string require_env(const char* name);
char jvm_classpath_separator();
void trace_step(const std::string& msg);
