# HLSL compiler
# Find DXC from Windows SDK and Vulkan SDK.

find_program(DXC 
	dxc
	PATHS $ENV{VULKAN_SDK}
	DOC "DirectX 12 shader compiler"
)
if ("${DXC}" STREQUAL "DXC-NOTFOUND")
	message(FATAL_ERROR "[Error]: DirectX Shader Compiler not found")
endif()
message(STATUS "[Info]: Found DirectX Shader Compiler - ${DXC}")