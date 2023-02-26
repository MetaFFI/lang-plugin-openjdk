project(lang-plugin-openjdk VERSION 0.0.1)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/compiler/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/runtime/)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/xllr-openjdk-bridge/)

