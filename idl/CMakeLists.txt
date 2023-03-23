
#------------------------------- "py" IDL Plugin ------------------------------------------
set(target_name "metaffi.idl.java")

add_java()
add_ctest_and_catch2_packages()

add_custom_target(${target_name} ALL)
add_dependencies(${target_name} metaffi.compiler.openjdk)
add_dependencies(${target_name} xllr.openjdk)

include(${CMAKE_SCRIPTS_DIR}/MetaFFI.cmake)

# Build JavaExtractor jar
FILE(GLOB JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/*.java)
list(REMOVE_ITEM JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/TestClass.java)
add_jar(JavaExtractor
		SOURCES ${JAVA_FILES}
		INCLUDE_JARS ${CMAKE_CURRENT_LIST_DIR}/javaparser-core-3.24.4.jar
		OUTPUT_DIR ${PROJECT_BINARY_DIR})

copy_file_from_project_binary_to_metaffi_home(JavaExtractor JavaExtractor.jar)

# Build java_extractor MetaFFI files
add_custom_target(${target_name}_link_java_extractor ALL)
add_dependencies(${target_name}_link_java_extractor ${target_name})
metaffi_compile_guest(${target_name}_link_java_extractor "JavaExtractor.json" WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
metaffi_compile_host(${target_name}_link_java_extractor "JavaExtractor.json" "go" WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_custom_target(${target_name}_build_openjdk_idl_plugin ALL)
add_dependencies(${target_name}_build_openjdk_idl_plugin ${target_name}_link_java_extractor)
go_get(${target_name}_build_openjdk_idl_plugin WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
go_build(${target_name}_build_openjdk_idl_plugin NAME ${target_name} WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

copy_file_post_build(${target_name}_build_openjdk_idl_plugin "${CMAKE_CURRENT_LIST_DIR}/javaparser-core-3.24.4.jar" "$ENV{METAFFI_HOME}/javaparser-core-3.24.4.jar")
copy_file_post_build(${target_name}_build_openjdk_idl_plugin "${CMAKE_CURRENT_LIST_DIR}/JavaExtractor_MetaFFIGuest.jar" "$ENV{METAFFI_HOME}/JavaExtractor_MetaFFIGuest.jar")
copy_file_post_build(${target_name}_build_openjdk_idl_plugin "${CMAKE_CURRENT_LIST_DIR}/JavaExtractor_MetaFFIGuest${CMAKE_SHARED_LIBRARY_SUFFIX}" "$ENV{METAFFI_HOME}/JavaExtractor_MetaFFIGuest${CMAKE_SHARED_LIBRARY_SUFFIX}")
copy_file_from_project_binary_to_metaffi_home(${target_name}_build_openjdk_idl_plugin ${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX})

copy_file_post_build(${target_name}_build_openjdk_idl_plugin "${CMAKE_CURRENT_LIST_DIR}/javaparser-core-3.24.4.jar" "${PROJECT_BINARY_DIR}/javaparser-core-3.24.4.jar")
copy_file_post_build(${target_name}_build_openjdk_idl_plugin "${CMAKE_CURRENT_LIST_DIR}/JavaExtractor_MetaFFIGuest.jar" "${PROJECT_BINARY_DIR}/JavaExtractor_MetaFFIGuest.jar")
copy_file_post_build(${target_name}_build_openjdk_idl_plugin "${CMAKE_CURRENT_LIST_DIR}/JavaExtractor_MetaFFIGuest${CMAKE_SHARED_LIBRARY_SUFFIX}" "${PROJECT_BINARY_DIR}/JavaExtractor_MetaFFIGuest${CMAKE_SHARED_LIBRARY_SUFFIX}")
copy_file_post_build(${target_name}_build_openjdk_idl_plugin "${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}" "${PROJECT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")

# Unitest
add_go_test("java IDL plugin test" WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

# install
install(FILES   ${PROJECT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}
				${PROJECT_BINARY_DIR}/JavaExtractor.jar
				${CMAKE_CURRENT_LIST_DIR}/javaparser-core-3.24.4.jar
				${CMAKE_CURRENT_LIST_DIR}/JavaExtractor_MetaFFIGuest.jar
				${CMAKE_CURRENT_LIST_DIR}/JavaExtractor_MetaFFIGuest${CMAKE_SHARED_LIBRARY_SUFFIX}
		DESTINATION .)
