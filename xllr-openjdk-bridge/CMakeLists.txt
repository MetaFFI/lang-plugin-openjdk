
#------------------------------- OpenJDK Language Support - Runtime Bridge ------------------------------------------
add_boost(filesystem)
add_jni()
add_java()

include(UseJava)

# set RPATH
set(target_jni_name "xllr.openjdk.jni.bridge")
set(target_jar_name "xllr.openjdk.bridge")

# build xllr.openjdk.jni.bridge DLL
get_dir_sources(${CMAKE_CURRENT_LIST_DIR} ${target_jni_name}_sources)
add_library(${target_jni_name} SHARED ${${target_jni_name}_sources})
target_include_directories(${target_jni_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} ${METAFFI_SDK} ${JNI_INCLUDE_DIRS})
set_target_properties(${target_jni_name} PROPERTIES PREFIX "" POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${target_jni_name} PRIVATE Boost::filesystem ${JNI_LIBRARIES} xllr.openjdk)
add_dependencies(${target_jni_name} xllr.openjdk)
link_to_dl_for_non_windows(${target_jni_name})

set_target_properties( ${target_jni_name}
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
		LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
		)

set_property(TARGET ${target_jni_name} APPEND PROPERTY INSTALL_RPATH "$ORIGIN;$ORIGIN/lib")
set_property(TARGET ${target_jni_name} APPEND PROPERTY BUILD_RPATH "$ORIGIN;$ORIGIN/lib")


# build xllr.openjdk.bridge
# compile java files and place in JAR
FILE(GLOB JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/*.java)
add_jar(${target_jar_name}
		SOURCES ${JAVA_FILES}
		OUTPUT_DIR ${PROJECT_BINARY_DIR})

# Copy to metaffi directory
copy_file_from_project_binary_to_metaffi_home(${target_jni_name} "${target_jni_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")
copy_file_from_project_binary_to_metaffi_home(${target_jar_name} "${target_jar_name}.jar")

# install
install_target(${target_jni_name} .)
install(FILES ${CMAKE_BINARY_DIR}/${target_jar_name}.jar DESTINATION .)
