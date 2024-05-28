cmake_minimum_required(VERSION 3.16.3)

# --- Global CMake header for all MetaFFI projects ---

# SDK Paths
set(METAFFI_SDK ${CMAKE_CURRENT_LIST_DIR}/plugin-sdk)
set(CMAKE_SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/plugin-sdk/cmake)

# add SDK
add_subdirectory(${METAFFI_SDK})

# VCPKG
add_vcpkg()

# policies
add_global_policies()


# set global C/C++ flags and compiler options
c_cpp_global_settings()

# set RPATH
add_rpath()

# --- Root project ---
project(lang-plugin-openjdk VERSION 0.0.1)

# global debug settings for all MetaFFI projects
c_cpp_debug_settings()

# --- Dependencies ---
add_ctest_and_unitest_libs()
add_boost(filesystem)
add_java()
add_jni()

# --- install dependencies ---
install_globals()
install_boost(filesystem)


# add targets
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/compiler/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/runtime/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/xllr-openjdk-bridge/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/api/)
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/idl/)
