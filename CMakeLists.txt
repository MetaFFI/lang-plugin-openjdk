cmake_minimum_required(VERSION 3.16.3)

if("$ENV{METAFFI_HOME}" STREQUAL "")
	message(FATAL_ERROR "METAFFI_HOME is missing - MetaFFI must be installed")
endif()

# policies
cmake_policy(SET CMP0110 NEW) # Allow arbitrary names in CTest names

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 20)
if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DEBUG) # add -DEBUG to "Debug" builds
	set(CMAKE_DEBUG_POSTFIX "")
endif()

# Add SDK
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/plugin-sdk/)
set(METAFFI_SDK ${CMAKE_CURRENT_LIST_DIR}/plugin-sdk)
set(CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/plugin-sdk/cmake)

# Load hunter package manager
load_hunter_pm(${CMAKE_SCRIPTS_DIR}) # load package manager

# include scripts required for project
include(${CMAKE_SCRIPTS_DIR}/InstallUtils.cmake)

#-----------------------------------------------------------------------------------------------

project(lang-plugin-openjdk VERSION 0.0.1)

# Add required packages
hunter_add_package(Catch)
hunter_add_package(doctest)
add_ctest_and_unitest_libs()

# set RPATH
set(CMAKE_MACOSX_RPATH 1) # tell MacOS RPATH is in use
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# add targets
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/compiler/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/runtime/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/xllr-openjdk-bridge/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/api/)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/idl/)

# cmake install (not cpack)
set(CMAKE_INSTALL_PREFIX [[$ENV{METAFFI_HOME}]])
set(CMAKE_SKIP_INSTALL_RPATH ON)

# package notes
set(INSTALL_NOTES "JVM and JNI must be installed to use this plugin.
Make sure JAVA_HOME environment variable is set.

Building with plugin: The file xllr.openjdk.bridge.jar, which is part of the plugin,
must be added to the classpath during build and execution.

Loading JVM module (using load()) expects the paths of the
generated native library and generated jar library delimited by semicolon.
")

file(WRITE ${PROJECT_BINARY_DIR}/notes.txt ${INSTALL_NOTES})

#-----------------------------------------------------------------------------------------------

