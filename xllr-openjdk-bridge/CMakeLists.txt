# load conan packages
find_or_install_package(Boost COMPONENTS filesystem)
find_or_install_package(doctest)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# build xllr.openjdk.jni.bridge C++ dynamic library
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} xllr.openjdk.jni.bridge)
c_cpp_shared_lib(xllr.openjdk.jni.bridge
		"${xllr.openjdk.jni.bridge_src};${sdk_src}"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS};"
		"Boost::filesystem;${JNI_LIBRARIES};xllr.openjdk"
		"./openjdk")

add_dependencies(xllr.openjdk.jni.bridge xllr.openjdk)
set(xllr.openjdk.jni.bridge xllr.openjdk.jni.bridge PARENT_SCOPE)

# copy jvm.dll and jawt.dll
# and usually not in the path
add_custom_command(TARGET xllr.openjdk.jni.bridge POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $ENV{JAVA_HOME}/bin/server/jvm${CMAKE_SHARED_LIBRARY_SUFFIX} $ENV{METAFFI_HOME}/openjdk/
)

add_custom_command(TARGET xllr.openjdk.jni.bridge POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $ENV{JAVA_HOME}/bin/jawt${CMAKE_SHARED_LIBRARY_SUFFIX} $ENV{METAFFI_HOME}/openjdk/
)

# build xllr.openjdk.bridge.jar from *.java
build_jar(xllr.openjdk.bridge
	SOURCES
		${CMAKE_CURRENT_LIST_DIR}/*.java
	OUTPUT_DIR
		"./openjdk/"
	OUTPUT_NAME
		xllr.openjdk.bridge
)
add_dependencies(xllr.openjdk.bridge xllr.openjdk.jni.bridge)
set(xllr.openjdk.bridge xllr.openjdk.bridge PARENT_SCOPE)