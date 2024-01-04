
#------------------------------- OpenJDK Language Support - Runtime Plugin ------------------------------------------

add_boost(filesystem)
add_jni()
add_java()

include(UseJava)

set(target_name "xllr.openjdk")

# build xllr.openjdk
get_dir_sources(${CMAKE_CURRENT_LIST_DIR} ${target_name}_sources)
get_sdk_utils_sources(utils_srcs)
get_sdk_runtime_sources(runtime_srcs)

add_library(${target_name} SHARED ${${target_name}_sources} ${utils_srcs} ${runtime_srcs})
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} ${METAFFI_SDK} ${JNI_INCLUDE_DIRS})
set_target_properties(${target_name} PROPERTIES PREFIX "" POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${target_name} PRIVATE Boost::filesystem ${JNI_LIBRARIES})
link_to_dl_for_non_windows(${target_name})

# set RPATH
set_property(TARGET ${target_name} APPEND PROPERTY INSTALL_RPATH "$ORIGIN;$ORIGIN/lib")
set_property(TARGET ${target_name} APPEND PROPERTY BUILD_RPATH "$ORIGIN;$ORIGIN/lib")

# Generate exports for XLLR
include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(${target_name}
		BASE_NAME ${target_name}
		EXPORT_MACRO_NAME ${target_name}_EXPORTS
		EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/xllr_openjdk_exports.h
		STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)

set_target_properties( ${target_name}
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
		LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
		)

# unitests
add_executable(openjdk_api_test "openjdk_api_test.cpp" ${utils_srcs} ${runtime_srcs})
target_link_libraries(openjdk_api_test PRIVATE Catch2::Catch2 ${target_name} Boost::filesystem ${JNI_LIBRARIES})
add_test(NAME "OpenJDK Runtime API" COMMAND openjdk_api_test)
target_include_directories(openjdk_api_test PRIVATE ${METAFFI_SDK} ${JNI_INCLUDE_DIRS})
if(NOT WIN32)
	add_custom_command(TARGET openjdk_api_test POST_BUILD COMMAND patchelf --set-rpath "$ORIGIN:$ORIGIN/lib:$ENV{METAFFI_HOME}:${Boost_LIBRARY_DIRS}:$ENV{JAVA_HOME}/lib/server/:$ENV{JAVA_HOME}/lib/:$ENV{JAVA_HOME}" $<TARGET_FILE:openjdk_api_test>)
endif()


# install
install_target(${target_name} .)
