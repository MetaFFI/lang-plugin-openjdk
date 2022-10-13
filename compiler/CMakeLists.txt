set(target_name "metaffi.compiler.lang.openjdk")

find_program(GOEXEC go)
if(NOT GOEXEC)
	message(FATAL "Go not found")
endif()

message("${target_name}")

# build
add_custom_target(${target_name} ALL)

add_custom_command(TARGET ${target_name}
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		COMMAND ${GOEXEC} build -buildmode=c-shared -gcflags=-shared -o ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}
		COMMENT "Building ${target_name} compiler plugin")


# Unitest TODO: RESUME TEST
#add_test(NAME compiler_openjdk_gotest
#		COMMAND ${GOEXEC} test
#		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# Copy to metaffi directory
add_custom_command(TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")
