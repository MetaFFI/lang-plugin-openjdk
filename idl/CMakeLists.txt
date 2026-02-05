# JVM IDL Plugin

# Find required dependencies
find_or_install_package(Boost COMPONENTS filesystem system)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# Collect C++ source files
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} metaffi_idl_jvm)

# Collect SDK JVM runtime_manager sources
collect_c_cpp_files("${metaffi_sdk_root}/runtime_manager/jvm" sdk_jvm_runtime_manager)

# Combined SDK sources for JVM
set(sdk_jvm_src
	${sdk_src}
	${sdk_jvm_runtime_manager_src}
)

# SDK include directories (add metaffi_sdk_root for <runtime_manager/jvm/...> style includes)
set(sdk_jvm_include
	${sdk_include_dir}
	${metaffi_sdk_root}
	${JNI_INCLUDE_DIRS}
)

# Build the IDL plugin as a dynamic library
# NO JVM link required - JVM is loaded dynamically at runtime
c_cpp_shared_lib(metaffi.idl.jvm
	"${metaffi_idl_jvm_src};${sdk_jvm_src}"
	"${sdk_jvm_include};${Boost_INCLUDE_DIRS}"
	"Boost::filesystem;Boost::system"
	"./jvm")

# Make plugin depend on JVM IDL compiler JAR
# The target jvm_idl_compiler is created by build_jar in sdk/idl_compiler/jvm/CMakeLists.txt
add_dependencies(metaffi.idl.jvm jvm_idl_compiler)

# Copy JVM IDL compiler dependencies to $METAFFI_HOME/jvm/idl
set(METAFFI_HOME_PATH $ENV{METAFFI_HOME})
if(NOT METAFFI_HOME_PATH)
	message(FATAL "METAFFI_HOME environment variable not set. JVM IDL compiler dependencies will not be copied.")
endif()

set(IDL_DEPS_DIR "${METAFFI_HOME_PATH}/jvm/idl")
set(IDL_COMPILER_JAR "${METAFFI_HOME_PATH}/sdk/idl_compiler/jvm/jvm_idl_compiler.jar")
set(IDL_SDK_LIB_DIR "${metaffi_sdk_root}/idl_compiler/jvm/lib")
set(ASM_JAR "${IDL_SDK_LIB_DIR}/asm-9.6.jar")
set(GSON_JAR "${IDL_SDK_LIB_DIR}/gson-2.10.1.jar")

if(NOT EXISTS ${ASM_JAR})
	message(FATAL_ERROR "Missing ASM dependency: ${ASM_JAR}")
endif()
if(NOT EXISTS ${GSON_JAR})
	message(FATAL_ERROR "Missing Gson dependency: ${GSON_JAR}")
endif()

add_custom_command(TARGET metaffi.idl.jvm POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E echo "Copying JVM IDL compiler dependencies to ${IDL_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${IDL_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${IDL_COMPILER_JAR}" "${IDL_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASM_JAR}" "${IDL_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSON_JAR}" "${IDL_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E echo "JVM IDL compiler dependencies copied successfully"
)

# Compile test data classes
add_custom_command(
    OUTPUT test_data_classes
    COMMAND ${Java_JAVAC_EXECUTABLE} -cp "libs/*" -d test/testdata test/testdata/*.java
    DEPENDS test/testdata/AllVariantsTest.java
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling test data classes"
)

# Java unit tests
set(JAVA_TEST_CLASSES_STAMP "${CMAKE_CURRENT_BINARY_DIR}/test/java_extractor/.classes_compiled")
add_custom_command(
    OUTPUT "${JAVA_TEST_CLASSES_STAMP}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/test/java_extractor"
    COMMAND ${Java_JAVAC_EXECUTABLE} -cp "libs/*;${CMAKE_CURRENT_BINARY_DIR}/src/java_extractor;test/testdata" -d "${CMAKE_CURRENT_BINARY_DIR}/test/java_extractor" test/java_extractor/*.java
    COMMAND ${CMAKE_COMMAND} -E touch "${JAVA_TEST_CLASSES_STAMP}"
    DEPENDS test/java_extractor/BytecodeExtractorTest.java test/java_extractor/JarExtractorTest.java test_data_classes jvm_idl_plugin_java_src
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling Java unit tests"
)

add_custom_target(jvm_idl_plugin_java_tests_classes
    DEPENDS "${JAVA_TEST_CLASSES_STAMP}"
)

# Run Java tests
add_custom_command(
    OUTPUT java_tests_run
    COMMAND ${Java_JAVA_EXECUTABLE} -cp "libs/*;${CMAKE_CURRENT_BINARY_DIR}/src/java_extractor;${CMAKE_CURRENT_BINARY_DIR}/test/java_extractor;test/testdata" org.junit.runner.JUnitCore java_extractor.BytecodeExtractorTest java_extractor.JarExtractorTest
    DEPENDS "${JAVA_TEST_CLASSES_STAMP}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Java unit tests"
)

# Copy JAR files to build directory
file(COPY 
    libs/junit-4.13.2.jar
    libs/hamcrest-core-1.3.jar
    libs/asm-9.6.jar
    libs/asm-tree-9.6.jar
    libs/javaparser-core-3.24.4.jar
    libs/gson-2.10.1.jar
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs
)

# Compile Java extractor sources to build directory
compile_java(jvm_idl_plugin_java_src
    SOURCES "src/java_extractor/*.java"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/src/java_extractor"
    CLASSPATH "libs/*"
    COMMENT "Compiling Java extractor sources"
)

# Add tests (JUnit)
add_junit_test(jvm_idl_plugin_java_tests
	TEST_CLASSES
		java_extractor.BytecodeExtractorTest
		java_extractor.JarExtractorTest
	CLASSPATH "libs/*" "${CMAKE_CURRENT_BINARY_DIR}/src/java_extractor" "${CMAKE_CURRENT_BINARY_DIR}/test/java_extractor" "test/testdata"
	OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/test"
	DEPENDS java_tests_run
	COMMENT "Running JUnit tests"
)

add_dependencies(metaffi.idl.jvm jvm_idl_plugin_java_tests_classes)

# Install rules - the c_cpp_shared_lib macro handles the copying to METAFFI_HOME

install(FILES 
    libs/junit-4.13.2.jar
    libs/hamcrest-core-1.3.jar
    libs/asm-9.6.jar
    libs/asm-tree-9.6.jar
    libs/javaparser-core-3.24.4.jar
    libs/gson-2.10.1.jar
    DESTINATION jvm/libs
)

install(DIRECTORY src/java_extractor/
    DESTINATION jvm/src/java_extractor
    FILES_MATCHING PATTERN "*.class"
) 
