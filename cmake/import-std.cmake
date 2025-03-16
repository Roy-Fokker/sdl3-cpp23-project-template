# Solution is experimental in CMake 3.31
cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

# Std Module Config
# Set the experimental flag
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
	"0e5b6991-d74f-4b3d-a41c-cf096e0b2508" # current value as per https://www.kitware.com/import-std-in-cmake-3-30/
)

# tell CMake we want to use 'import std'
# will get enabled for all targets declared after this
set(CMAKE_CXX_MODULE_STD 1)