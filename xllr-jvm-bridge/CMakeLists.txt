# load conan packages
find_or_install_package(Boost COMPONENTS filesystem)
find_or_install_package(doctest)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# build xllr.jvm.jni.bridge C++ dynamic library
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} xllr.jvm.jni.bridge)
c_cpp_shared_lib(xllr.jvm.jni.bridge
		"${xllr.jvm.jni.bridge_src};${sdk_src}"
		"${sdk_include_dir};${Boost_INCLUDE_DIRS};${JNI_INCLUDE_DIRS};"
		"Boost::filesystem;${JNI_LIBRARIES};xllr.jvm"
		"./jvm")

if(TARGET xllr.jvm)
	add_dependencies(xllr.jvm.jni.bridge xllr.jvm)
endif()
set(xllr.jvm.jni.bridge xllr.jvm.jni.bridge PARENT_SCOPE)

# build xllr.jvm.bridge.jar from *.java
build_jar(xllr.jvm.bridge
	SOURCES
		${CMAKE_CURRENT_LIST_DIR}/*.java
	OUTPUT_DIR
		"jvm"
	OUTPUT_NAME
		xllr.jvm.bridge
)
add_dependencies(xllr.jvm.bridge xllr.jvm.jni.bridge)
set(xllr.jvm.bridge xllr.jvm.bridge PARENT_SCOPE)
