# Use CPM.CMake dependency manager
include(${CMAKE_MODULE_PATH}/CPM.cmake)

# Dependencies
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.20")

CPMAddPackage(
	URI "gh:g-truc/glm#master"
	OPTIONS                        # have to manually specify options
	"GLM_BUILD_TESTS OFF"          # else it chooses to build Shared Library
	"BUILD_STATIC_LIBS ON"         # This does not export .lib file with MSVC
	"BUILD_SHARED_LIBS OFF"        # Seem GLM does not export any symbols (dllexport)
)
