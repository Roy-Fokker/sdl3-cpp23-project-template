# SDL3 + C++23 Project Template
---
Template project to play around with SDL3 GPU API with C++23 using modules.

## Using the template
### Project Config
- In root `CMakeLists.txt` change `project name`, `url`, and `description`.
- In `src/CMakeLists.txt` change `PRJ_APP_NAME`
- Ensure `dxc` shader compiler is findable by CMake, 
  - `target_hlsl_sources` cmake function accepts shader format as option.
    It will choose DXC location based on shader format in `src/CMakeLists.txt`
	Supports either SPIRV or DXIL (DXIL is windows only)

### CMake commands for configuring and building
VSCode will run these automatically.
- On Windows
```shell
  # Configure Project
  cmake --preset windows-default
  # Build Project, parameter order matters
  cmake --build --preset windows-debug
```
- On Linux
```shell
  # Configure Project
  cmake --preset linux-default
  # Build Project, parameter order matters
  cmake --build --preset linux-debug
```

## Uses/Dependencies
- CPM.CMake for package management
- Ninja-Build for build engine
- CMake 3.31+ with cmakepresets for configuration and build
  - There is an issue, on my machine, with CMake 4.0. It fails to configure properly.
  - CMake 4.0.1 fixed the issue with configure, seems like it was regression in CMake.
- C++ modules enabled
- Uses C++ Standard Library modules
  - MSVC (Windows only)
  - Clang (Linux with libc++ only)
- Focuses on SDL3 GPU
- HLSL for all shaders, compile to SPIRV for Vulkan and DXIL for Direct3D

## Example project dependencies
Uses following libraries retrieved from their project repos by CPM.CMake
- SDL3, obviously
- GLM, for math
- DXC, from Vulkan SDK for Vulkan backend **or** from Windows SDK for DirectX backend
- Dear Imgui, for simple debug ui

## Basic notes on code structure
- As much as is possible, functions will not take ownership of pointers to objects.
- Where able, all SDL GPU types are wrapped into std::unique_ptr with custom deleters, so they self clean on scope exit.
- SDL init and quit is wrapped in C++ class and created/destructed by Application class.
- `Application` class does following
  - Create SDL Window, GPU, Imgui, in Application
  - Create Basic Pipeline, in `Prepare Scene`
    - Simple vertex shader that uses uniform buffer to fix rendering perspective
    - Simple pixel/fragment shader
  - Vertex and Index buffers for Square mesh, in `Prepare Scene`
  - Create Perspective projection, in `Prepare Scene`
  - Create ImGui window in `Update Scene`
  - `Draw`s square using Indexed Primitives
  - `Draw` gui via ImGui
  - Quit on any-key pressed, in `Handle SDL Input`

## To be figured out
- Intellisense and clangd both cannot handle modules so don't work correctly
  - Hacky solution for intellisense to show STD library types.

## references
- CMake 3.30 magic incantations, https://www.kitware.com/import-std-in-cmake-3-30/
- CMake module compilations, https://www.kitware.com/import-cmake-the-experiment-is-over/
- CMake experimental flag GUID for modules, https://github.com/Kitware/CMake/blob/master/Help/dev/experimental.rst#c-import-std-support

