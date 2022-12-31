find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library

find_package(JNI REQUIRED) # JNI library
find_package(Java 11 REQUIRED COMPONENTS Development)
include(UseJava)

set(target_jni_name "xllr.openjdk.jni.bridge")
set(target_jar_name "xllr.openjdk.bridge")

# build xllr_java_bridge_src DLL
FILE(GLOB CPP_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${METAFFI_SDK}/utils/*.cpp)
add_library(${target_jni_name} SHARED ${CPP_FILES})
target_include_directories(${target_jni_name} PUBLIC ${JNI_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR})
set_property(TARGET ${target_jni_name} PROPERTY POSITION_INDEPENDENT_CODE ON)
SET_TARGET_PROPERTIES(${target_jni_name} PROPERTIES PREFIX "")

target_link_libraries(${target_jni_name} Boost::filesystem ${JNI_LIBRARIES} xllr.openjdk)

add_dependencies(${target_jni_name} xllr.openjdk)

if(WIN32)
	target_link_libraries(${target_jni_name} ${JNI_LIBRARIES})
	target_link_directories(${target_jni_name} PUBLIC ${BOOST_LIBRARYDIR})
else()
	target_link_libraries(${target_jni_name} dl Boost::filesystem ${JNI_LIBRARIES})
endif()

# compile java files and place in JAR
FILE(GLOB JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/*.java)
add_jar(${target_jar_name}
		SOURCES ${JAVA_FILES} )


# Copy to metaffi directory
add_custom_command(TARGET ${target_jni_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{METAFFI_HOME}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{METAFFI_HOME}/${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")

add_custom_command(TARGET ${target_jar_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_jar_name}.jar"
		"$ENV{METAFFI_HOME}/${target_jar_name}.jar"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_jar_name}.jar -> $ENV{METAFFI_HOME}/${target_jar_name}.jar")

# install
install(TARGETS ${target_jni_name} DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_jar_name}.jar DESTINATION .)

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
	ADD_DEB_DEPENDS(${CMAKE_BINARY_DIR} ${target_jni_name} "python|boost|expat|jvm" INSTALL_FILES INSTALL_RESOLVED_FILES INSTALL_FILES_ERROR)

	if (${INSTALL_FILES_ERROR})
		message( FATAL_ERROR "Dependencies files resolution failed: " ${INSTALL_FILES_ERROR} )
	endif (${INSTALL_FILES_ERROR})

	if(INSTALL_FILES)
		install(FILES ${INSTALL_FILES} # Copy all libraries and symlinks pointing to real paths
				DESTINATION .)
	endif()

	if(INSTALL_RESOLVED_FILES)
		install(FILES ${INSTALL_RESOLVED_FILES} # Copy all libraries real paths
				DESTINATION .)
	endif()
endif()