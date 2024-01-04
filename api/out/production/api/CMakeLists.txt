
#------------------------------- OpenJDK Language Support - Runtime Bridge ------------------------------------------
include(UseJava)

# set RPATH
set(target_jar_name "metaffi.api")

add_jar(${target_jar_name}
		SOURCES MetaFFIModule.java MetaFFIRuntime.java
		INCLUDE_JARS ${CMAKE_CURRENT_LIST_DIR}/lib/hamcrest-core-1.3.jar
		INCLUDE_JARS ${CMAKE_CURRENT_LIST_DIR}/lib/junit-4.13.1.jar
		INCLUDE_JARS $ENV{METAFFI_HOME}/xllr.openjdk.bridge.jar
		OUTPUT_DIR ${PROJECT_BINARY_DIR})

# Copy to metaffi directory
copy_file_from_project_binary_to_metaffi_home(${target_jar_name} "${target_jar_name}.jar")

# install
install(FILES ${CMAKE_BINARY_DIR}/${target_jar_name}.jar DESTINATION .)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/lib/junit-4.13.1.jar DESTINATION .)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/lib/hamcrest-core-1.3.jar DESTINATION .)
