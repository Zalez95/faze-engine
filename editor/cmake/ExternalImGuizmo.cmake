include(FetchContent)

FetchContent_Declare(
	ImGuizmo
	GIT_REPOSITORY https://github.com/Zalez95/imguizmo.git
	GIT_TAG cmake
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(ImGuizmo)
if(NOT imguizmo_POPULATED)
	FetchContent_Populate(ImGuizmo)

	set(IMGUIZMO_BUILD_EXAMPLES OFF CACHE INTERNAL "")
	if(FORCE_STATIC_VCRT)
		set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE INTERNAL "")
	else()
		set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE INTERNAL "")
	endif()
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE INTERNAL "")
	set(CMAKE_BUILD_TYPE ${CONFIG} CACHE INTERNAL "")
	set(CMAKE_DEBUG_POSTFIX ${MY_DEBUG_POSTFIX} CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE INTERNAL "")

	add_subdirectory(${imguizmo_SOURCE_DIR} ${imguizmo_BINARY_DIR})
endif()