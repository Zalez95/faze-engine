include(ExternalProject)

# Download the project
ExternalProject_Add(FreeTypeDownload
	DOWNLOAD_COMMAND	git submodule update --init "${EXTERNAL_PATH}/freetype"
	SOURCE_DIR			"${EXTERNAL_PATH}/freetype"
	INSTALL_DIR			"${EXTERNAL_INSTALL_PATH}/freetype"
	CMAKE_ARGS			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
						-DCMAKE_BUILD_TYPE=$<CONFIG>
						-DCMAKE_DEBUG_POSTFIX=${MY_DEBUG_POSTFIX}
						-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
						-DFORCE_STATIC_VCRT=${FORCE_STATIC_VCRT}
						--no-warn-unused-cli
)

# Get the properties from the downloaded target
ExternalProject_Get_Property(FreeTypeDownload INSTALL_DIR)

set(FREETYPE_FOUND TRUE)
set(FREETYPE_INCLUDE_DIR "${INSTALL_DIR}/include/freetype2")
set(FREETYPE_LIBRARY_DIR "${INSTALL_DIR}/lib/")
if(BUILD_SHARED_LIBS)
	set(FREETYPE_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}freetype${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(FREETYPE_DEBUG_LIBRARY "${CMAKE_SHARED_LIBRARY_PREFIX}freetype${MY_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(FREETYPE_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}freetype${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(FREETYPE_DEBUG_LIBRARY "${CMAKE_STATIC_LIBRARY_PREFIX}freetype${MY_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Create the target and add its properties
add_library(FreeType INTERFACE)
target_include_directories(FreeType INTERFACE ${FREETYPE_INCLUDE_DIR})
target_link_libraries(FreeType INTERFACE
	optimized "${FREETYPE_LIBRARY_DIR}${FREETYPE_LIBRARY}"
	debug "${FREETYPE_LIBRARY_DIR}${FREETYPE_DEBUG_LIBRARY}"
)
add_dependencies(FreeType FreeTypeDownload)
