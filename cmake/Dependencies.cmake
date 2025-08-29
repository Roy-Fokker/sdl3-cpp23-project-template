# Use CPM.CMake dependency manager
include(${CMAKE_MODULE_PATH}/CPM.cmake)

# Dependencies
## Simple Directmedia Library
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.20")

## Graphics Math Library
CPMAddPackage(
	URI "gh:g-truc/glm#master"
	OPTIONS                        # have to manually specify options
	"GLM_BUILD_TESTS OFF"          # else it chooses to build Shared Library
	"BUILD_STATIC_LIBS ON"         # This does not export .lib file with MSVC
	"BUILD_SHARED_LIBS OFF"        # Seem GLM does not export any symbols (dllexport)
)

## Dear ImGui Library
CPMAddPackage(
	URI "gh:ocornut/imgui#v1.92.2b"
	DOWNLOAD_ONLY TRUE              # don't build or do anything extra just download source
)
if (imgui_ADDED)
	add_library(imgui STATIC)
	add_library(imgui::imgui ALIAS imgui)

	target_sources(imgui                                              # Only adding source files to imgui library
		PRIVATE                                                       # which are necessary for SDL3 GPU backend
			${imgui_SOURCE_DIR}/imgui.cpp                             # functions.
			${imgui_SOURCE_DIR}/imgui_draw.cpp                        # Other backends are not going to get used
			${imgui_SOURCE_DIR}/imgui_tables.cpp                      # so no point in including them.
			${imgui_SOURCE_DIR}/imgui_widgets.cpp                     # 
			${imgui_SOURCE_DIR}/imgui_demo.cpp                        # 
			${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp          # Backend implementation of imgui using
			${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp       # SDL3 GPU functionality
	)

	target_include_directories(imgui      # make sure directories of imgui
		PUBLIC                            # are findable by both imgui and 
			${imgui_SOURCE_DIR}           # consuming project.
			${imgui_SOURCE_DIR}/backends  # for including headers
	)

	target_compile_definitions(imgui
		PUBLIC
			IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	)

	target_link_libraries(imgui
		PRIVATE
			SDL3::SDL3
	)

	message(STATUS "IMGUI: added library as imgui::imgui")
endif()
