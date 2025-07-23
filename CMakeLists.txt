
# add SDK
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/plugin-sdk")

# idl plugin
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/idl")

# runtime
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/runtime")

# bridge
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/xllr-openjdk-bridge")

# api
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/api")

add_custom_target(openjdk
	DEPENDS xllr.openjdk xllr.openjdk.jni.bridge xllr.openjdk.bridge metaffi.api openjdk_api_test cdts_java_test openjdk_idl_plugin openjdk_idl_plugin_test
)

set(openjdk ${openjdk} PARENT_SCOPE)
