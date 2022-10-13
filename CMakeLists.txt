cmake_minimum_required(VERSION 3.16.3)
project(metaffi.lang.openjdk VERSION 0.0.1)

include(CTest)
enable_testing()

set(METAFFI_SDK ${CMAKE_CURRENT_LIST_DIR}/../plugin-sdk)
set(METAFFI_CORE ${CMAKE_CURRENT_LIST_DIR}/../metaffi-core)
include(${METAFFI_SDK}/CMakeLists.txt)

include(${CMAKE_CURRENT_LIST_DIR}/compiler/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/xllr-openjdk-bridge/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/runtime/CMakeLists.txt)
