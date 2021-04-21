find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library
find_package(JNI REQUIRED) # JNI library
find_package(Java 11 REQUIRED COMPONENTS Development)
include(UseJava)

set(target_jni_name "xllr.openjdk.jni.bridge")
set(target_jar_name "xllr.openjdk.bridge")

# build xllr_java_bridge_src DLL
FILE(GLOB CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_library(${target_jni_name} SHARED ${CPP_FILES})
target_include_directories(${target_jni_name} PUBLIC ${JNI_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR} ${OPENFFI_SDK}/utils/*.cpp)
set_property(TARGET ${target_jni_name} PROPERTY POSITION_INDEPENDENT_CODE ON)
SET_TARGET_PROPERTIES(${target_jni_name} PROPERTIES PREFIX "")
target_link_libraries(${target_jni_name} Boost::filesystem ${JNI_LIBRARIES})

# compile java files and place in JAR
FILE(GLOB JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/*.java)
add_jar(${target_jar_name}
		SOURCES ${JAVA_FILES} )

# Copy to openffi directory
add_custom_command(TARGET ${target_jni_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{OPENFFI_HOME}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{OPENFFI_HOME}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_custom_command(TARGET ${target_jar_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_jar_name}.jar"
		"$ENV{OPENFFI_HOME}/${target_jar_name}.jar"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_jar_name}.jar -> $ENV{OPENFFI_HOME}/${target_jar_name}.jar")
