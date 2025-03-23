
# add SDK
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/plugin-sdk")



# runtime
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/runtime")

# bridge
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/xllr-openjdk-bridge")

# api
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/api")

add_custom_target(openjdk
	DEPENDS xllr.openjdk xllr.openjdk.jni.bridge xllr.openjdk.bridge metaffi.api openjdk_api_test cdts_java_test
)

set(openjdk ${openjdk} PARENT_SCOPE)
