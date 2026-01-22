# JVM Compiler Plugin

# Build the compiler plugin using go_build macro
go_build(metaffi.compiler.jvm
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_DIR jvm
    OUTPUT_NAME metaffi.compiler.jvm
    DEPENDENT ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add Go tests using add_go_test macro
add_go_test(jvm_compiler_go_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
) 