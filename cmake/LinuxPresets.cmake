cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

# Set Properties for Linux specific configurations
LIST(APPEND platform_definitions
	UNICODE _UNICODE            # Tell compiler we are using UNICODE
	LINUX                       # We are building for Linux
)

# Set Compiler flags for MSVC
# Not used atm, as preset set same options for msvc
LIST(APPEND compiler_options
	-stdlib=libc++                     # use LibC++ as GCC doesn't support modules
)

