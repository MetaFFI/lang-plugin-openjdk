
# add SDK
if(NOT DEFINED sdk_src)
	message(FATAL_ERROR "sdk_src is not defined. Ensure sdk/CMakeLists.txt is added before lang-plugin-jvm.")
endif()

# idl plugin
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/idl")

# compiler plugin
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/compiler")

# runtime
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/runtime")

# bridge
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/xllr-jvm-bridge")

# api
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/api")

add_custom_target(jvm
	DEPENDS xllr.jvm xllr.jvm.jni.bridge xllr.jvm.bridge metaffi.api jvm_api_test cdts_java_test metaffi.idl.jvm metaffi.compiler.jvm
)

set(jvm ${jvm} PARENT_SCOPE)
