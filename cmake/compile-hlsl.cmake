# HLSL compiler
# Find DXC from Windows SDK and Vulkan SDK.

find_program(DXC 
	NAMES dxc dxc.exe
	HINTS $ENV{VULKAN_SDK}/bin /usr/bin
	PATHS $ENV{VULKAN_SDK}/bin /usr/bin
	DOC "DirectX 12 shader compiler"
	NO_DEFAULT_PATH
)
if ("${DXC}" STREQUAL "DXC-NOTFOUND")
	message(FATAL_ERROR "[Error]: DirectX Shader Compiler not found")
endif()
message(STATUS "[Info]: Found DirectX Shader Compiler - ${DXC}")