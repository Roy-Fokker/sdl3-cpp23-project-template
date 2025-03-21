cmake_minimum_required(VERSION 3.31)

# Enable scan for dependencies
set(CXX_SCAN_FOR_MODULES ON)
# disable C++ extensions
set(CMAKE_CXX_EXTENSIONS OFF)
# force C++ version to 23
set(CMAKE_CXX_STANDARD 23)
# standard is required
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Std Modules cmake 
# must be included BEFORE project() call
include(${CMAKE_MODULE_PATH}/import-std.cmake)

# Include/Download Dependencies
include(${CMAKE_MODULE_PATH}/Dependencies.cmake)

# Include functions to enable compiling HLSL shaders
include(${CMAKE_MODULE_PATH}/compile-hlsl.cmake)

# project and language to be used
project(sdl3-cpp23-project-template
	LANGUAGES CXX
	HOMEPAGE_URL "https://roy-fokker.github.io/sdl3-cpp23-project-template/"
	DESCRIPTION "SDL3 + C++23 based Project Template.")

# executable source folder
add_subdirectory(src)