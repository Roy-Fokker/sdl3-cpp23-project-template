# SDL3 + C++23 Project Template

## Uses
- CPM.CMake for package management
- Ninja-Build for build engine
- CMake 3.31+ with cmakepresets for configuration and build
  - There is an issue, on my machine, with CMake 4.0. It fails to configure properly.
- C++ modules enabled
- Uses C++ Standard Library modules
  - MSVC (Windows only)
  - Clang (Linux with libc++ only)
- Focuses on SDL3 GPU
- HLSL for all shaders

## to be figured out
- Intellisense and clangd both cannot handle modules so don't work correctly
- Should be able to change from CPM.CMake to vcpkg

## CMake commands
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

## Example project dependencies
Uses following libraries retrieved from their project repos by CPM.CMake
- SDL3, obviously
- GLM, for math
- DXC, either from Windows SDK (for D3D12 backend) or Vulkan SDK (for Vulkan backend)

## references
- Building on linux with Clang and libc++ for module support, https://mattbolitho.github.io/posts/vcpkg-with-libcxx/
- CMake 3.30 magic encantations, https://www.kitware.com/import-std-in-cmake-3-30/
- CMake module compilations, https://www.kitware.com/import-cmake-the-experiment-is-over/
