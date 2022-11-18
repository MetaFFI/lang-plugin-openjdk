find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library
find_package(JNI REQUIRED) # JNI library
find_package(Java 11 REQUIRED COMPONENTS Development)

include(CTest)
enable_testing()

set(target_name "xllr.openjdk")

# build xllr.openjdk
file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp
		${CMAKE_CURRENT_LIST_DIR}/*.h
		${CMAKE_CURRENT_LIST_DIR}/*.hpp
		${METAFFI_SDK}/utils/*.cpp
		${METAFFI_SDK}/runtime/cdts_wrapper.*
		${METAFFI_SDK}/runtime/cdt_capi_loader.c)

add_library(${target_name}
			SHARED
			${SRC_FILES}
)
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS})

set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# remove the "lib" prefix in linux to get the expected filename
SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "")

if(WIN32)
	target_link_libraries(${target_name} Boost::filesystem ${JNI_LIBRARIES})
else()
	target_link_libraries(${target_name} dl Boost::filesystem ${JNI_LIBRARIES})
endif()

# Copy to metaffi directory
add_custom_command(TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")


# Unitest
add_executable(jvm_guest_test ${CMAKE_CURRENT_LIST_DIR}/jvm_guest_test.cpp)
target_link_libraries(jvm_guest_test ${target_name} xllr.test)
add_test(NAME jvm_guest_test COMMAND jvm_guest_test)

