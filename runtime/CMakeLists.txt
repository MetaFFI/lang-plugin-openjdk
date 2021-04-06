find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library

include(CTest)
enable_testing()

# TODO: change to your language
set(target_name "xllr.[your language]")

# build xllr.go
file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.h ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
add_library(${target_name}
			SHARED
			${SRC_FILES}
)
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS})

set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# remove the "lib" prefix in linux to get the expected filename
SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "")

target_link_libraries(${target_name} Boost::filesystem)

# Copy to openffi directory
add_custom_command(TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{OPENFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{OPENFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")


# Unitest go
add_executable(xllr_test ${CMAKE_CURRENT_LIST_DIR}/xllr_test.cpp)
target_link_libraries(xllr_test Boost::filesystem dl)
add_test(NAME test_go_module_success COMMAND xllr_go_test 1)
add_test(NAME test_go_module_free_module_via_free_runtime_success COMMAND xllr_go_test 2)
add_test(NAME test_go_module_lazy_runtime_success COMMAND xllr_go_test 3)
add_test(NAME test_go_module_module_not_exist_fail COMMAND xllr_go_test 4)
add_test(NAME test_go_call_success COMMAND xllr_go_test 5)
add_test(NAME test_go_call_fail COMMAND xllr_go_test 6)
