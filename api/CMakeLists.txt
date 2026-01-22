find_or_install_package(Boost COMPONENTS filesystem)
find_or_install_package(doctest)


# get all java files in CMAKE_CURRENT_LIST_DIR
file(GLOB JAVA_FILES ${CMAKE_CURRENT_LIST_DIR}/*.java)

# * ---- build metaffi.api.jar from *.java ----
build_jar(metaffi.api
	SOURCES
		${JAVA_FILES}
	INCLUDE_JARS
		$ENV{METAFFI_HOME}/jvm/xllr.jvm.bridge.jar
	OUTPUT_DIR
		"./jvm/"
	OUTPUT_NAME
		metaffi.api
)
set(metaffi.api metaffi.api PARENT_SCOPE)
add_dependencies(metaffi.api xllr.jvm.bridge)

# * ---- Cross Language tests ----
add_test(NAME jvm_api_cross_pl_tests
		COMMAND ${PYTHON_EXECUTABLE} ${PYTHON_EXECUTABLE_ARG} run_api_tests.py
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
