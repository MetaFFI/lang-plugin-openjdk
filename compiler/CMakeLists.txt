# Compiler Plugin for JVM
# Builds a dynamic library that loads the JVM dynamically and implements the compiler plugin interface

# Find required dependencies
find_or_install_package(Boost COMPONENTS filesystem system)
find_package(JNI REQUIRED)

# Collect C++ source files from this directory
collect_c_cpp_files(${CMAKE_CURRENT_LIST_DIR} metaffi_compiler_jvm)

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

# Build the compiler plugin as a dynamic library
# NO JVM link required - JVM is loaded dynamically at runtime
c_cpp_shared_lib(metaffi.compiler.jvm
	"${metaffi_compiler_jvm_src};${sdk_jvm_src}"
	"${sdk_jvm_include};${Boost_INCLUDE_DIRS}"
	"Boost::filesystem;Boost::system"
	"./jvm")

# Copy JVM compiler dependencies to $METAFFI_HOME/jvm/compiler
set(METAFFI_HOME_PATH $ENV{METAFFI_HOME})
if(NOT METAFFI_HOME_PATH)
	message(FATAL "METAFFI_HOME environment variable not set. JVM compiler dependencies will not be copied.")
endif()

set(COMPILER_DEPS_DIR "${METAFFI_HOME_PATH}/jvm/compiler")
set(JVM_HOST_COMPILER_JAR_NAME "metaffi.jvm.compiler.host")
set(JVM_HOST_COMPILER_JAR "${METAFFI_HOME_PATH}/jvm/compiler/${JVM_HOST_COMPILER_JAR_NAME}.jar")
set(JVM_IDL_ENTITIES_JAR "${METAFFI_HOME_PATH}/sdk/idl_entities/jvm/jvm_idl_entities.jar")
set(GSON_JAR "${metaffi_sdk_root}/idl_compiler/jvm/lib/gson-2.10.1.jar")

if(NOT EXISTS ${GSON_JAR})
	message(FATAL_ERROR "Missing Gson dependency: ${GSON_JAR}")
endif()

add_custom_command(TARGET metaffi.compiler.jvm POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E echo "Copying JVM compiler dependencies to ${COMPILER_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${COMPILER_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${JVM_HOST_COMPILER_JAR}" "${COMPILER_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${JVM_IDL_ENTITIES_JAR}" "${COMPILER_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GSON_JAR}" "${COMPILER_DEPS_DIR}"
	COMMAND ${CMAKE_COMMAND} -E echo "JVM compiler dependencies copied successfully"
)

add_dependencies(metaffi.compiler.jvm jvm_host_compiler jvm_idl_entities)

set(metaffi_compiler_jvm metaffi.compiler.jvm PARENT_SCOPE)
