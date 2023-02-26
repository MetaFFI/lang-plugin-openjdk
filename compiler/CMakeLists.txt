
#------------------------------- OpenJDK Language Support - Compiler Plugin ------------------------------------------
project(lang-plugin-openjdk-compiler VERSION 0.0.1)
set(target_name "metaffi.compiler.lang.openjdk")

# build
add_go_target(${target_name})

go_get(${target_name})
go_build(${target_name})
add_go_test("OpenJDK compiler test")

copy_file_from_project_binary_to_metaffi_home(${target_name} ${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX})

# install
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}
		DESTINATION .)