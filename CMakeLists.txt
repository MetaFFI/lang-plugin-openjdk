cmake_minimum_required(VERSION 3.17.3)
project(openffi.lang.openjdk VERSION 0.0.1)

include(CTest)
enable_testing()

include(${CMAKE_CURRENT_LIST_DIR}/../plugin-sdk/CMakeLists.txt)

include(compiler/CMakeLists.txt)
include(runtime/CMakeLists.txt)