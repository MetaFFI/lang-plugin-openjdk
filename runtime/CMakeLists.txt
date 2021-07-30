find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library
find_package(JNI REQUIRED) # JNI library
find_package(Java 11 REQUIRED COMPONENTS Development)

include(CTest)
enable_testing()

set(target_name "xllr.openjdk")

# build xllr.go
file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.h ${CMAKE_CURRENT_LIST_DIR}/*.hpp ${METAFFI_SDK}/utils/*.cpp)
add_library(${target_name}
			SHARED
			${SRC_FILES}
)
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS})

set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# remove the "lib" prefix in linux to get the expected filename
SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "")

target_link_libraries(${target_name} Boost::filesystem ${JNI_LIBRARIES})

# Copy to metaffi directory
add_custom_command(TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")


# Unitest
#add_executable(xllr_test ${CMAKE_CURRENT_LIST_DIR}/xllr_test.cpp)
#target_link_libraries(xllr_test Boost::filesystem dl)
#add_test(NAME test_openjdk_module_success COMMAND xllr_openjdk_test 1)
#add_test(NAME test_openjdk_module_free_module_via_free_runtime_success COMMAND xllr_openjdk_test 2)
#add_test(NAME test_openjdk_module_lazy_runtime_success COMMAND xllr_openjdk_test 3)
#add_test(NAME test_openjdk_module_module_not_exist_fail COMMAND xllr_openjdk_test 4)
#add_test(NAME test_openjdk_call_success COMMAND xllr_openjdk_test 5)
#add_test(NAME test_openjdk_call_fail COMMAND xllr_openjdk_test 6)
