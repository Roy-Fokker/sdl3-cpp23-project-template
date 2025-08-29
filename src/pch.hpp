#pragma once

// #include "pch_std.hpp"

#include <cassert>

#include <SDL3/SDL.h> // SDL header

#include <glm/glm.hpp> // Required for glm::vec3/4/mat4/etc
#include <glm/ext.hpp> // Required for glm::perspective function

#include <imgui.h>              // IMGUI primary header
#include <imgui_impl_sdl3.h>    // IMGUI's SDL3 implementation
#include <imgui_impl_sdlgpu3.h> // IMGUI's SDL-GPU implementation