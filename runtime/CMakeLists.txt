find_package(Boost REQUIRED COMPONENTS filesystem) # Boost library
find_package(JNI REQUIRED) # JNI library
find_package(Java 11 REQUIRED COMPONENTS Development)

include(CTest)
enable_testing()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(target_name "xllr.openjdk")

# build xllr.openjdk
file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp
		${CMAKE_CURRENT_LIST_DIR}/*.h
		${CMAKE_CURRENT_LIST_DIR}/*.hpp
		${METAFFI_SDK}/utils/*.cpp
		${METAFFI_SDK}/runtime/cdts_wrapper.*
		${METAFFI_SDK}/runtime/cdt_capi_loader.c)

add_library(${target_name}
			SHARED
			${SRC_FILES}
)
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS} ${JNI_INCLUDE_DIRS})

set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# remove the "lib" prefix in linux to get the expected filename
SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "")

if(WIN32)
	target_link_libraries(${target_name} ${JNI_LIBRARIES})
	target_link_directories(${target_name} PUBLIC ${BOOST_LIBRARYDIR})
else()
	target_link_libraries(${target_name} dl Boost::filesystem ${JNI_LIBRARIES})
endif()

include(GenerateExportHeader)
GENERATE_EXPORT_HEADER(xllr.openjdk
		BASE_NAME xllr.openjdk
		EXPORT_MACRO_NAME XLLR_OPENJDK_EXPORTS
		EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/xllr_openjdk_exports.h
		STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC)

# Copy to metaffi directory
add_custom_command(TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"$ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		COMMENT "Copy ${CMAKE_CURRENT_BINARY_DIR}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX} -> $ENV{METAFFI_HOME}/${target_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")


# install
install(TARGETS ${target_name} DESTINATION .)

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
	ADD_DEB_DEPENDS(${CMAKE_BINARY_DIR} ${target_name} "python|boost|expat|jvm" INSTALL_FILES INSTALL_RESOLVED_FILES INSTALL_FILES_ERROR)

	if (${INSTALL_FILES_ERROR})
		message( FATAL_ERROR "Dependencies files resolution failed: " ${INSTALL_FILES_ERROR} )
	endif (${INSTALL_FILES_ERROR})

	if(INSTALL_FILES)
		install(FILES ${INSTALL_FILES} # Copy all libraries and symlinks pointing to real paths
				DESTINATION .)
	endif()

	if(INSTALL_RESOLVED_FILES)
		install(FILES ${INSTALL_RESOLVED_FILES} # Copy all libraries real paths
				DESTINATION .)
	endif()
endif()