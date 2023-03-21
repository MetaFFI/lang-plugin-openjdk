
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

# install
install_target(${target_name} .)
