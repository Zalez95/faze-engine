include(ExternalProject)

set(GTEST_WARNING_FLAGS "")
if(CMAKE_CXX_COMPILER MATCHES "MSVC")
	set(GTEST_WARNING_FLAGS "/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

# Download the project
ExternalProject_Add(gtestDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EXTERNAL_PATH}/gtest"
	SOURCE_DIR			"${EXTERNAL_PATH}/gtest"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/gtest"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
						-DCMAKE_DEBUG_POSTFIX=${MY_DEBUG_POSTFIX}
						-DCMAKE_CXX_FLAGS=${GTEST_WARNING_FLAGS}
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
						-DBUILD_GTEST=ON
						-DBUILD_GMOCK=OFF
						-Dgtest_force_shared_crt=$<NOT:$<BOOL:${FORCE_STATIC_VCRT}>>
						-Wno-dev
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(gtestDownload INSTALL_DIR)

set(GTEST_FOUND TRUE)
set(GTEST_INCLUDE_DIR "${INSTALL_DIR}/include/")
set(GTEST_LIBRARY_DIR "${INSTALL_DIR}/lib/")
if(BUILD_SHARED_LIBS)
	set(GTEST_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}gtest${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(GTEST_DEBUG_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}gtest${MY_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(GTEST_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(GTEST_DEBUG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}gtest${MY_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(gtest INTERFACE)
target_include_directories(gtest INTERFACE ${GTEST_INCLUDE_DIR})
target_link_libraries(gtest INTERFACE
	optimized "${GTEST_LIBRARY_DIR}${GTEST_LIBRARY}"
	debug "${GTEST_LIBRARY_DIR}${GTEST_DEBUG_LIBRARY}"
)
add_dependencies(gtest gtestDownload)
