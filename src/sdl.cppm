module;

export module sdl;

import std;

export namespace sdl
{
	namespace type
	{
		// If we are building in DEBUG mode, use this variable to enable extra messages from SDL
		constexpr auto IS_DEBUG = bool{
#ifdef DEBUG
			true
#endif
		};

		// Deleter template, for use with SDL objects.
		// Allows use of SDL Objects with C++'s smart pointers, using SDL's destroy function
		template <auto fn>
		struct sdl_deleter
		{
			constexpr void operator()(auto *arg)
			{
				fn(arg);
			}
		};
		// Define SDL types with std::unique_ptr and custom deleter
		using gpu_ptr    = std::unique_ptr<SDL_GPUDevice, sdl_deleter<SDL_DestroyGPUDevice>>;
		using window_ptr = std::unique_ptr<SDL_Window, sdl_deleter<SDL_DestroyWindow>>;

		template <auto fn>
		struct gpu_deleter
		{
			SDL_GPUDevice *gpu = nullptr;
			constexpr void operator()(auto *arg)
			{
				fn(gpu, arg);
			}
		};
		// Define SDL GPU types with std::unique_ptr and custom deleter
		using free_gfx_pipeline  = gpu_deleter<SDL_ReleaseGPUGraphicsPipeline>;
		using gfx_pipeline_ptr   = std::unique_ptr<SDL_GPUGraphicsPipeline, free_gfx_pipeline>;
		using free_comp_pipeline = gpu_deleter<SDL_ReleaseGPUComputePipeline>;
		using comp_pipeline_ptr  = std::unique_ptr<SDL_GPUComputePipeline, free_comp_pipeline>;
		using free_gfx_shader    = gpu_deleter<SDL_ReleaseGPUShader>;
		using gfx_shader_ptr     = std::unique_ptr<SDL_GPUShader, free_gfx_shader>;
		using free_gpu_buffer    = gpu_deleter<SDL_ReleaseGPUBuffer>;
		using gpu_buffer_ptr     = std::unique_ptr<SDL_GPUBuffer, free_gpu_buffer>;
		using free_gpu_texture   = gpu_deleter<SDL_ReleaseGPUTexture>;
		using gpu_texture_ptr    = std::unique_ptr<SDL_GPUTexture, free_gpu_texture>;
		using free_gpu_sampler   = gpu_deleter<SDL_ReleaseGPUSampler>;
		using gfx_sampler_ptr    = std::unique_ptr<SDL_GPUSampler, free_gpu_sampler>;
	}

	auto make_window(uint32_t width, uint32_t height, std::string_view title, SDL_WindowFlags flags = {}) -> type::window_ptr
	{
		auto window = SDL_CreateWindow(title.data(), static_cast<int>(width), static_cast<int>(height), flags);
		assert(window != nullptr and "Window could not be created.");

		// enable relative mouse movement
		SDL_SetWindowRelativeMouseMode(window, true);

		return type::window_ptr{ window };
	}

	auto make_gpu(SDL_GPUShaderFormat preferred_shader_format) -> type::gpu_ptr
	{
		auto gpu = SDL_CreateGPUDevice(preferred_shader_format, type::IS_DEBUG, NULL);
		assert(gpu != nullptr and "GPU device could not be created.");

		return type::gpu_ptr{ gpu };
	}
}