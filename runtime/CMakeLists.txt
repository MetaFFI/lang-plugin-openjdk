# load conan packages
find_or_install_package(Boost COMPONENTS filesystem)
find_or_install_package(doctest)
find_or_install_package(Java REQUIRED)
find_or_install_package(JNI REQUIRED)

collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} xllr.openjdk)

# get cpython from "deps" as vcpkg doesn't have python3.11 and python3.12
c_cpp_shared_lib(xllr.openjdk
		"${xllr.openjdk_src};${sdk_src}"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS};"
		"Boost::filesystem;${JNI_LIBRARIES}"
		"./openjdk")

set(xllr.openjdk xllr.openjdk PARENT_SCOPE)

# run cdts_java_test.cpp doctest unit test
c_cpp_exe(cdts_java_test
		"${xllr.openjdk_src};${sdk_src};cdts_java_test.cpp"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${doctest_INCLUDE_DIRS};${JNI_INCLUDE_DIRS}"
		"doctest::doctest;Boost::filesystem;${JNI_LIBRARIES}"
		".")
add_dependencies(cdts_java_test xllr.openjdk)
add_test(NAME cdts_java_test COMMAND $ENV{METAFFI_HOME}/cdts_java_test)
set(cdts_java_test cdts_java_test PARENT_SCOPE)

c_cpp_exe(openjdk_api_test
		"${xllr.openjdk_src};${sdk_src};cdts_java_test.cpp"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${doctest_INCLUDE_DIRS};${JNI_INCLUDE_DIRS}"
		"doctest::doctest;Boost::filesystem;${JNI_LIBRARIES}"
		".")
add_dependencies(openjdk_api_test xllr.openjdk)
add_test(NAME openjdk_api_test COMMAND $ENV{METAFFI_HOME}/openjdk_api_test)
set(openjdk_api_test openjdk_api_test PARENT_SCOPE)