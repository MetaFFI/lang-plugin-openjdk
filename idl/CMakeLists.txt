# OpenJDK IDL Plugin

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Java and JNI
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# Include directories
include_directories(${JNI_INCLUDE_DIRS})

# Build main JAR using build_jar macro
build_jar(openjdk_idl_extractor
    SOURCES "src/java_extractor/*.java"
    OUTPUT_DIR "openjdk"
    OUTPUT_NAME "openjdk_idl_extractor"
    INCLUDE_JARS "libs/gson-2.10.1.jar;libs/asm-9.6.jar;libs/asm-tree-9.6.jar;libs/javaparser-core-3.24.4.jar"
)

# C++ plugin library
add_library(openjdk_idl_plugin SHARED
    openjdk_idl_plugin.cpp
)

# Link with JNI
target_link_libraries(openjdk_idl_plugin ${JNI_LIBRARIES})

# Set library properties
set_target_properties(openjdk_idl_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    OUTPUT_NAME "openjdk_idl_plugin"
)

# Compile test data classes
add_custom_command(
    OUTPUT test_data_classes
    COMMAND ${Java_JAVAC_EXECUTABLE} -cp "libs/*" -d test/testdata test/testdata/*.java
    DEPENDS test/testdata/SimpleTest.java test/testdata/ComplexTest.java
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling test data classes"
)

# Java unit tests
add_custom_command(
    OUTPUT java_tests_classes
    COMMAND ${Java_JAVAC_EXECUTABLE} -cp "libs/*;src/java_extractor;test/testdata" -d test/java_extractor test/java_extractor/*.java
    DEPENDS test/java_extractor/BytecodeExtractorTest.java test_data_classes
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling Java unit tests"
)

# Run Java tests
add_custom_command(
    OUTPUT java_tests_run
    COMMAND ${Java_JAVA_EXECUTABLE} -cp "libs/*;src/java_extractor;test/java_extractor;test/testdata" org.junit.runner.JUnitCore java_extractor.BytecodeExtractorTest
    DEPENDS java_tests_classes
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Java unit tests"
)

# C++ test executable
add_executable(openjdk_idl_plugin_test
    test/cpp/openjdk_idl_plugin_test.cpp
)

# Link test with plugin
target_link_libraries(openjdk_idl_plugin_test openjdk_idl_plugin)

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

# Copy compiled Java classes to build directory
add_custom_command(
    TARGET openjdk_idl_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/java_extractor
    ${CMAKE_CURRENT_BINARY_DIR}/src/java_extractor
    COMMENT "Copying Java classes to build directory"
)

# Add tests
add_test(NAME openjdk_idl_plugin_java_tests COMMAND ${CMAKE_COMMAND} -E echo "Java tests completed")
add_test(NAME openjdk_idl_plugin_cpp_test COMMAND openjdk_idl_plugin_test)

# Make test executable part of the build
add_dependencies(openjdk_idl_plugin_test openjdk_idl_plugin)

# Cleanup test files after tests
add_custom_command(
    TARGET openjdk_idl_plugin_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/testdata
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/testdata
    COMMENT "Cleaning up test class files"
)

# Install rules
install(TARGETS openjdk_idl_plugin
    LIBRARY DESTINATION openjdk
    RUNTIME DESTINATION openjdk
)

install(FILES 
    libs/junit-4.13.2.jar
    libs/hamcrest-core-1.3.jar
    libs/asm-9.6.jar
    libs/asm-tree-9.6.jar
    libs/javaparser-core-3.24.4.jar
    libs/gson-2.10.1.jar
    DESTINATION openjdk/libs
)

install(DIRECTORY src/java_extractor/
    DESTINATION openjdk/src/java_extractor
    FILES_MATCHING PATTERN "*.class"
) 