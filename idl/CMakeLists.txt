# JVM IDL Plugin

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Java and JNI
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

# Include directories
include_directories(${JNI_INCLUDE_DIRS})

# C++ plugin library using the c_cpp_shared_lib macro
# Depends on JVM IDL compiler classes being built first
c_cpp_shared_lib(metaffi.idl.jvm
    "jvm_idl_plugin.cpp"
    "${JNI_INCLUDE_DIRS};${sdk_include_dir}"
    "${JNI_LIBRARIES}"
    "jvm"
)

# Make plugin depend on JVM IDL compiler JAR
# The target jvm_idl_compiler is created by build_jar in sdk/idl_compiler/jvm/CMakeLists.txt
add_dependencies(metaffi.idl.jvm jvm_idl_compiler)

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
    DEPENDS test/java_extractor/BytecodeExtractorTest.java test/java_extractor/JarExtractorTest.java test_data_classes
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling Java unit tests"
)

# Run Java tests
add_custom_command(
    OUTPUT java_tests_run
    COMMAND ${Java_JAVA_EXECUTABLE} -cp "libs/*;src/java_extractor;test/java_extractor;test/testdata" org.junit.runner.JUnitCore java_extractor.BytecodeExtractorTest java_extractor.JarExtractorTest
    DEPENDS java_tests_classes
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

# Copy compiled Java classes to build directory
add_custom_command(
    TARGET metaffi.idl.jvm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/java_extractor
    ${CMAKE_CURRENT_BINARY_DIR}/src/java_extractor
    COMMENT "Copying Java classes to build directory"
)

# Add tests
add_test(NAME jvm_idl_plugin_java_tests 
    COMMAND ${Java_JAVA_EXECUTABLE} -cp "libs/*;src/java_extractor;test/java_extractor;test/testdata" org.junit.runner.JUnitCore java_extractor.BytecodeExtractorTest java_extractor.JarExtractorTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_tests_properties(jvm_idl_plugin_java_tests PROPERTIES
    DEPENDS java_tests_run
)

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
