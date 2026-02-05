# load conan packages
find_or_install_package(Boost COMPONENTS filesystem system)

# Collect plugin source files
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} xllr.jvm)

# Collect SDK jvm runtime_manager sources
collect_c_cpp_files("${metaffi_sdk_root}/runtime_manager/jvm" sdk_jvm_runtime_manager)

# Collect SDK jvm cdts_serializer sources
collect_c_cpp_files("${metaffi_sdk_root}/cdts_serializer/jvm" sdk_jvm_cdts_serializer)

# Combined SDK sources for jvm
set(sdk_jvm_src
	${sdk_src}
	${sdk_jvm_runtime_manager_src}
	${sdk_jvm_cdts_serializer_src}
)

# SDK include directories (add metaffi_sdk_root for <runtime_manager/jvm/...> style includes)
set(sdk_jvm_include
	${sdk_include_dir}
	${metaffi_sdk_root}
)

# Build shared library
c_cpp_shared_lib(xllr.jvm
		"${xllr.jvm_src};${sdk_jvm_src}"
		"${sdk_jvm_include};${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS}"
		"Boost::filesystem;Boost::system;${JNI_LIBRARIES}"
		"./jvm")

set(xllr.jvm xllr.jvm PARENT_SCOPE)
