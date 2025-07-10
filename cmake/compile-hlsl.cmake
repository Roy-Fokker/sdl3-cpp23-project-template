#---------------------------------------------------------------------------------------
# Function to take shader file and compile it as dependency of program

function(target_hlsl_sources TARGET SHADER_FORMAT)
	# HLSL compiler
	# Find DXC from Vulkan SDK or Windows SDK.
	if (SHADER_FORMAT MATCHES "SPIRV")
		find_program(DXC 
			NAMES dxc dxc.exe
			HINTS $ENV{VULKAN_SDK}/bin /usr/bin
			PATHS $ENV{VULKAN_SDK}/bin /usr/bin
			DOC "DirectX 12 shader compiler from Vulkan SDK"
			NO_CACHE
			NO_DEFAULT_PATH
		)
	elseif(SHADER_FORMAT MATCHES "DXIL")
		find_program(DXC 
			NAMES dxc dxc.exe
			DOC "DirectX 12 shader compiler from Windows SDK"
			NO_CACHE
		)
	endif()

	if ("${DXC}" STREQUAL "DXC-NOTFOUND")
		message(FATAL_ERROR "[Error]: DirectX Shader Compiler not found")
	endif()
	message(STATUS "[Info]: Found DirectX Shader Compiler - ${DXC}")
	message(STATUS "[Info]: Compiling Shaders in ${SHADER_FORMAT} format")

	# figure out how many files we have to configure given the pattern
	list(LENGTH ARGN count_HLSL)
	math(EXPR count_HLSL "${count_HLSL} / 3")

	# List of compiled shader output
	set(shader_files "")
	set(shader_sources "")

	# Are we using Vulkan SDK's dxc?
	set(use_spirv "")
	if (SHADER_FORMAT MATCHES "SPIRV")
		set(use_spirv "-spirv")
	endif()
	
	# Are we in debug mode?
	string(TOLOWER ${CMAKE_BUILD_TYPE} compile_mode)
	if (${compile_mode} STREQUAL "debug")
		list(APPEND shader_pdb_options /Zi)

		# Are we using Windows SDK's dxc?
		if (SHADER_FORMAT MATCHES "DXIL")
			list(APPEND shader_pdb_options /Fd ${CMAKE_PDB_OUTPUT_DIRECTORY}/)
		endif()
	endif()
	
	# Loop through all the pairs for filename:profile provided
	foreach(i RANGE 1 ${count_HLSL})
		math(EXPR fni "(${i}-1)*3")              # filename index
		math(EXPR pfi "${fni}+2")                # profile index
		list(GET ARGN ${fni} hlsl_filename)      # get the filename[i]
		list(GET ARGN ${pfi} hlsl_profile)       # get the profile[i]

		# get the absolute path of current source file
		file(REAL_PATH ${hlsl_filename} source_abs)

		if(NOT EXISTS ${source_abs})
			message(FATAL_ERROR "Cannot find shader file: ${source_abs}")
		endif()

		# get only the filename from absolute path
		cmake_path(GET source_abs STEM basename)
		set(basename "${basename}.${hlsl_profile}")

		# get only the parent directory of the file from absolute path
		cmake_path(GET source_abs PARENT_PATH source_fldr)
		get_filename_component(source_fldr "${source_fldr}" NAME)
		
		# shader output folder will be a subfolder in the binary directory
		set(shader_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

		# full path to compiled output 
		set(output ${shader_dir}/${basename}.cso)

		# call vulkan sdk's dxc compiler with source and output arguments.
		add_custom_command(
			OUTPUT ${output}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${shader_dir}
			COMMAND ${DXC} ${use_spirv} -E main -Fo ${output} -T ${hlsl_profile} ${source_abs} ${shader_pdb_options}
			DEPENDS ${source_abs}
			COMMENT "DXC Compiling SPIRV: ${hlsl_filename} -> ${output}"
			VERBATIM
		)

		list(APPEND shader_sources "${hlsl_filename}")
		list(APPEND shader_files "${output}")
	endforeach()

	# make a new variable to hold all output target names
	set(shader_group "${TARGET}_HLSL")
	# add custom target using new variable bound to output file of glslc step
	add_custom_target("${shader_group}"
					  DEPENDS "${shader_files}"
					  SOURCES "${shader_sources}"
	)

	# add compilation of this shader as dependency of the target
	add_dependencies("${TARGET}" "${shader_group}")
endfunction()