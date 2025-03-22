# get platform specific configs
if(MSVC)
	include(${CMAKE_MODULE_PATH}/Win32Presets.cmake)
elseif(LINUX)
	include(${CMAKE_MODULE_PATH}/LinuxPresets.cmake)
endif()

# name of this application/library
set(PRJ_APP_NAME "sdl3-cpp-app")

# create executable for initial setup
add_executable(${PRJ_APP_NAME})

# ensure it's Standard C++ 23
target_compile_features(${PRJ_APP_NAME} PRIVATE cxx_std_23)

# set properties for GLM basically want this anytime it is used
LIST(APPEND glm_definitions
	GLM_FORCE_DEPTH_ZERO_TO_ONE # GLM clip space should be in Z-axis to 0 to 1
	GLM_FORCE_LEFT_HANDED       # GLM should use left-handed coordinates, +z goes into screen
	GLM_FORCE_RADIANS           # GLM should always use radians not degrees.
)

# set preprocessor defines
target_compile_definitions(${PRJ_APP_NAME}
	PRIVATE
		$<$<CONFIG:Debug>:DEBUG>    # Ensure DEBUG macro is defined in debug build
		${platform_definitions}     # Get platform specific definitions
		${glm_definitions}          # Definitions to configure GLM
)

# precompiled headers
target_precompile_headers(${PRJ_APP_NAME}
	PRIVATE
		pch.hpp
)

# source files for this application
target_sources(${PRJ_APP_NAME}
	# Non C++ module source files
	PRIVATE
		main.cpp

	# magic encantations for C++ Modules Support in CMake
	PRIVATE FILE_SET app_modules TYPE CXX_MODULES FILES
		sdl.cppm
		application.cppm
		clock.cppm
		io.cppm
)

# libraries used by this application
target_link_libraries(${PRJ_APP_NAME}
	PRIVATE
		SDL3::SDL3 # SDL3 Library
		glm::glm   # GL Math Library
)

# shaders used by this project
target_hlsl_sources(${PRJ_APP_NAME}
	shaders/basic.vs.hlsl : vs_6_4
	shaders/basic.ps.hlsl : ps_6_4
)