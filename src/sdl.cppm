module;

export module sdl;

import std;
import io;

namespace rg = std::ranges;
namespace vw = std::views;

export namespace sdl
{
	// If we are building in DEBUG mode, use this variable to enable extra messages from SDL
	constexpr auto IS_DEBUG = bool{
#ifdef DEBUG
		true
#endif
	};

	// SDL requires call init and quit functions.
	// This class does that, it only needs to init in class.
	// Object should be created before any other SDL type.
	class sdl_base
	{
	public:
		sdl_base()
		{
			auto result = SDL_Init(SDL_INIT_VIDEO);
			assert(result and "SDL could not be initialized.");
		}

		~sdl_base()
		{
			SDL_Quit();
		}
	};

	namespace type
	{
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
		// Define SDL type with std::unique_ptr and custom deleter
		using window_ptr = std::unique_ptr<SDL_Window, sdl_deleter<SDL_DestroyWindow>>;

		// Special deleter for gpu.
		// it will release window on destruction
		struct gpu_window_deleter
		{
			SDL_Window *window;
			constexpr void operator()(auto *gpu)
			{
				SDL_ReleaseWindowFromGPUDevice(gpu, window);
				SDL_DestroyGPUDevice(gpu);
			}
		};
		// Define GPU type with std::unique_ptr and custom deleter
		using gpu_ptr = std::unique_ptr<SDL_GPUDevice, gpu_window_deleter>;

		// Deleter for all gpu objects in SDL
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

	// make_gpu does not take ownership of *wnd. *wnd must stay alive for duration of gpu_ptr.
	auto make_gpu(SDL_Window *wnd, SDL_GPUShaderFormat preferred_shader_format) -> type::gpu_ptr
	{
		auto gpu = SDL_CreateGPUDevice(preferred_shader_format, IS_DEBUG, NULL);
		assert(gpu != nullptr and "GPU device could not be created.");

		std::println("GPU Driver API: {}", SDL_GetGPUDeviceDriver(gpu));

		auto result = SDL_ClaimWindowForGPUDevice(gpu, wnd);
		assert(result == true and "Could not claim window for GPU.");

		return type::gpu_ptr{ gpu, { wnd } };
	}

	auto next_swapchain_image(SDL_Window *wnd, SDL_GPUCommandBuffer *cmd_buf) -> SDL_GPUTexture *
	{
		auto sc_tex = (SDL_GPUTexture *)nullptr;

		auto res = SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buf, wnd, &sc_tex, NULL, NULL);
		assert(res == true and "Wait and acquire GPU swapchain texture failed.");
		assert(sc_tex != nullptr and "Swapchain texture is null. Is window minimized?");

		return sc_tex;
	}

	enum class shader_stage : uint8_t
	{
		invalid,
		vertex,
		fragment,
		compute,
	};

	// Does not convert shader_stage::compute
	// as graphics pipeline cannot use that value, and SDL does not provide SDL_GPUShaderStage value for it.
	auto to_sdl(shader_stage stage) -> SDL_GPUShaderStage
	{
		switch (stage)
		{
		case shader_stage::vertex:
			return SDL_GPU_SHADERSTAGE_VERTEX;
		case shader_stage::fragment:
			return SDL_GPU_SHADERSTAGE_FRAGMENT;
		}
		assert(false and "Unhandled shader stage");
		return {};
	}

	enum class raster_type : uint8_t
	{
		none_fill,
		none_wire,
		front_ccw_fill,
		front_ccw_wire,
		back_ccw_fill,
		back_ccw_wire,
		front_cw_fill,
		front_cw_wire,
		back_cw_fill,
		back_cw_wire,
	};

	auto to_sdl(raster_type type) -> SDL_GPURasterizerState
	{
		switch (type)
		{
		case raster_type::none_fill:
			return {
				.fill_mode = SDL_GPU_FILLMODE_FILL,
				.cull_mode = SDL_GPU_CULLMODE_NONE,
			};
		case raster_type::none_wire:
			return {
				.fill_mode = SDL_GPU_FILLMODE_LINE,
				.cull_mode = SDL_GPU_CULLMODE_NONE,
			};
		case raster_type::front_ccw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case raster_type::front_ccw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case raster_type::back_ccw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case raster_type::back_ccw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
			};
		case raster_type::front_cw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		case raster_type::front_cw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_FRONT,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		case raster_type::back_cw_fill:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_FILL,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		case raster_type::back_cw_wire:
			return {
				.fill_mode  = SDL_GPU_FILLMODE_LINE,
				.cull_mode  = SDL_GPU_CULLMODE_BACK,
				.front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
			};
		}
		assert(false and "Unhandled raster type");
		return {};
	}

	enum class blend_type : uint8_t
	{
		none,
		opaque,
		alpha,
		additive,
		non_premultiplied,
	};

	auto to_sdl(blend_type type) -> SDL_GPUColorTargetBlendState
	{
		// TODO: verify and fix blend ops for different types.

		auto src    = SDL_GPUBlendFactor{ SDL_GPU_BLENDFACTOR_ONE };
		auto dst    = SDL_GPUBlendFactor{ SDL_GPU_BLENDFACTOR_ONE };
		auto op     = SDL_GPUBlendOp{ SDL_GPU_BLENDOP_ADD };
		auto enable = false;

		switch (type)
		{
		case blend_type::opaque:
			src = SDL_GPU_BLENDFACTOR_ONE;
			dst = SDL_GPU_BLENDFACTOR_ZERO;
			break;
		case blend_type::alpha:
			src = SDL_GPU_BLENDFACTOR_ONE;
			dst = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case blend_type::additive:
			src = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst = SDL_GPU_BLENDFACTOR_ONE;
			break;
		case blend_type::non_premultiplied:
			src = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		}

		enable = ((src != SDL_GPU_BLENDFACTOR_ONE) or (dst != SDL_GPU_BLENDFACTOR_ONE));

		return {
			.src_color_blendfactor = src,
			.dst_color_blendfactor = dst,
			.color_blend_op        = op,
			.src_alpha_blendfactor = src,
			.dst_alpha_blendfactor = dst,
			.alpha_blend_op        = op,
			.enable_blend          = enable,
		};
	}

	auto get_gpu_supported_shader_format(SDL_GPUDevice *gpu) -> SDL_GPUShaderFormat
	{
		auto backend_formats = SDL_GetGPUShaderFormats(gpu);

		if (backend_formats & SDL_GPU_SHADERFORMAT_DXIL)
		{
			return SDL_GPU_SHADERFORMAT_DXIL;
		}

		if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV)
		{
			return SDL_GPU_SHADERFORMAT_SPIRV;
		}

		// TODO: add other backends

		return SDL_GPU_SHADERFORMAT_INVALID;
	}

	auto get_gpu_supported_depth_stencil_format(SDL_GPUDevice *gpu) -> SDL_GPUTextureFormat
	{
		// Order Matters, biggest to smallest
		constexpr auto depth_formats = std::array{
			SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
			SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
			SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
			SDL_GPU_TEXTUREFORMAT_D24_UNORM,
			SDL_GPU_TEXTUREFORMAT_D16_UNORM,
		};

		auto rng = depth_formats | vw::filter([&](const auto fmt) {
			return SDL_GPUTextureSupportsFormat(gpu, fmt, SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET);
		}) | vw::take(1);

		assert(rng.begin() != rng.end() and "None of the depth formats are supported.");

		return *rng.begin();
	}

	struct shader_builder
	{
		io::byte_array shader_binary   = {};
		shader_stage stage             = shader_stage::invalid;
		uint32_t sampler_count         = 0;
		uint32_t uniform_buffer_count  = 0;
		uint32_t storage_uniform_count = 0;
		uint32_t storage_texture_count = 0;

		auto build(SDL_GPUDevice *gpu) const -> type::gfx_shader_ptr
		{
			assert(shader_binary.size() != 0 and "Shader binary is empty.");

			auto shader_info = SDL_GPUShaderCreateInfo{
				.code_size            = shader_binary.size(),
				.code                 = reinterpret_cast<const uint8_t *>(shader_binary.data()),
				.entrypoint           = "main",
				.format               = get_gpu_supported_shader_format(gpu),
				.stage                = to_sdl(stage),
				.num_samplers         = sampler_count,
				.num_storage_textures = storage_texture_count,
				.num_storage_buffers  = storage_uniform_count,
				.num_uniform_buffers  = uniform_buffer_count,
			};

			auto shader = SDL_CreateGPUShader(gpu, &shader_info);
			assert(shader != nullptr and "Failed to create shader.");

			return { shader, { gpu } };
		}
	};

	enum class topology_type : uint8_t
	{
		triangle_list,
		triangle_strip,
		line_list,
		line_strip,
		point_list,
	};

	auto to_sdl(topology_type type) -> SDL_GPUPrimitiveType
	{
		switch (type)
		{
		case topology_type::triangle_list:
			return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		case topology_type::triangle_strip:
			return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
		case topology_type::line_list:
			return SDL_GPU_PRIMITIVETYPE_LINELIST;
		case topology_type::line_strip:
			return SDL_GPU_PRIMITIVETYPE_LINESTRIP;
		case topology_type::point_list:
			return SDL_GPU_PRIMITIVETYPE_POINTLIST;
		}
		assert(false and "Unhandled topology type");
		return {};
	}

	struct gfx_pipeline_builder
	{
		type::gfx_shader_ptr vertex_shader   = nullptr;
		type::gfx_shader_ptr fragment_shader = nullptr;

		std::span<const SDL_GPUVertexAttribute> vertex_attributes                  = {};
		std::span<const SDL_GPUVertexBufferDescription> vertex_buffer_descriptions = {};

		SDL_GPUTextureFormat color_format = {};

		bool enable_depth_stencil = false;

		raster_type raster     = {};
		blend_type blend       = {};
		topology_type topology = {};

		auto build(SDL_GPUDevice *gpu) -> type::gfx_pipeline_ptr
		{
			auto vertex_input_state = SDL_GPUVertexInputState{
				.vertex_buffer_descriptions = vertex_buffer_descriptions.data(),
				.num_vertex_buffers         = static_cast<uint32_t>(vertex_buffer_descriptions.size()),
				.vertex_attributes          = vertex_attributes.data(),
				.num_vertex_attributes      = static_cast<uint32_t>(vertex_attributes.size()),
			};

			auto depth_stencil_state  = SDL_GPUDepthStencilState{};
			auto depth_stencil_format = SDL_GPUTextureFormat{};

			// Surpringly AMD doesn't support D24 on Vulkan, it does on DirectX??
			if (enable_depth_stencil)
			{
				depth_stencil_state = SDL_GPUDepthStencilState{
					.compare_op          = SDL_GPU_COMPAREOP_LESS,
					.write_mask          = std::numeric_limits<uint8_t>::max(),
					.enable_depth_test   = true,
					.enable_depth_write  = true,
					.enable_stencil_test = false, // TODO: figure out how to enable stencil under current api
				};

				depth_stencil_format = get_gpu_supported_depth_stencil_format(gpu);
			}

			auto color_targets = std::array{
				SDL_GPUColorTargetDescription{
				  .format      = color_format,
				  .blend_state = to_sdl(blend),
				},
			};

			auto target_info = SDL_GPUGraphicsPipelineTargetInfo{
				.color_target_descriptions = color_targets.data(),
				.num_color_targets         = static_cast<uint32_t>(color_targets.size()),
				.depth_stencil_format      = depth_stencil_format,
				.has_depth_stencil_target  = enable_depth_stencil,
			};

			auto pipeline_info = SDL_GPUGraphicsPipelineCreateInfo{
				.vertex_shader       = vertex_shader.get(),
				.fragment_shader     = fragment_shader.get(),
				.vertex_input_state  = vertex_input_state,
				.primitive_type      = to_sdl(topology),
				.rasterizer_state    = to_sdl(raster),
				.depth_stencil_state = depth_stencil_state,
				.target_info         = target_info,
			};

			auto pipeline = SDL_CreateGPUGraphicsPipeline(gpu, &pipeline_info);
			assert(pipeline != nullptr and "Failed to create graphics pipeline.");

			return { pipeline, { gpu } };
		}
	};

	auto make_gpu_buffer(SDL_GPUDevice *gpu, SDL_GPUBufferUsageFlags usage, uint32_t size, std::string_view debug_name) -> type::gpu_buffer_ptr
	{
		auto buffer_info = SDL_GPUBufferCreateInfo{
			.usage = usage,
			.size  = size,
		};

		auto buffer = SDL_CreateGPUBuffer(gpu, &buffer_info);
		assert(buffer != nullptr and "Failed to create gpu buffer");

		if (IS_DEBUG and debug_name.size() > 0)
		{
			SDL_SetGPUBufferName(gpu, buffer, debug_name.data());
		}

		return { buffer, { gpu } };
	}

	auto make_gpu_texture(SDL_GPUDevice *gpu, const SDL_GPUTextureCreateInfo &texture_info, std::string_view debug_name) -> type::gpu_texture_ptr
	{
		auto ti = texture_info;
#ifdef WINDOWS
		if (get_gpu_supported_shader_format(gpu) & SDL_GPU_SHADERFORMAT_DXIL and
		    texture_info.usage & SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET)
		{
			auto props = SDL_CreateProperties();
			SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, 1.0f);
			ti.props = props;
		}
#endif

		auto texture = SDL_CreateGPUTexture(gpu, &ti);
		assert(texture != nullptr and "Failed to create gpu texture.");

		if (IS_DEBUG and debug_name.size() > 0)
		{
			SDL_SetGPUTextureName(gpu, texture, debug_name.data());
		}

		return { texture, { gpu } };
	}

	void upload_to_gpu(SDL_GPUDevice *gpu, SDL_GPUBuffer *buffer, io::byte_span src_data)
	{
		auto src_size = static_cast<uint32_t>(src_data.size());

		auto transfer_info = SDL_GPUTransferBufferCreateInfo{
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size  = src_size,
		};
		auto transfer_buffer = SDL_CreateGPUTransferBuffer(gpu, &transfer_info);
		assert(transfer_buffer != nullptr and "Failed to create transfer buffer.");

		auto dst_data = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, false);
		std::memcpy(dst_data, src_data.data(), src_size);
		SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);

		auto copy_cmd = SDL_AcquireGPUCommandBuffer(gpu);
		{
			auto copy_pass = SDL_BeginGPUCopyPass(copy_cmd);
			{
				auto src = SDL_GPUTransferBufferLocation{
					.transfer_buffer = transfer_buffer,
					.offset          = 0,
				};

				auto dst = SDL_GPUBufferRegion{
					.buffer = buffer,
					.offset = 0,
					.size   = src_size,
				};

				SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);
			}
			SDL_EndGPUCopyPass(copy_pass);
			SDL_SubmitGPUCommandBuffer(copy_cmd);
		}
		SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
	}

	// void upload_to_gpu(SDL_GPUDevice *gpu, SDL_GPUTexture *texture, const io::image_t &img)
	// {
	// 	auto src_size = static_cast<uint32_t>(img.data.size());

	// 	auto transfer_info = SDL_GPUTransferBufferCreateInfo{
	// 		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
	// 		.size  = src_size,
	// 	};
	// 	auto transfer_buffer = SDL_CreateGPUTransferBuffer(gpu, &transfer_info);
	// 	assert(transfer_buffer != nullptr and "Failed to create transfer buffer.");

	// 	auto dst_data = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, false);
	// 	std::memcpy(dst_data, img.data.data(), src_size);
	// 	SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);

	// 	auto copy_cmd = SDL_AcquireGPUCommandBuffer(gpu);
	// 	{
	// 		auto copy_pass = SDL_BeginGPUCopyPass(copy_cmd);
	// 		{
	// 			auto src = SDL_GPUTextureTransferInfo{
	// 				.transfer_buffer = transfer_buffer,
	// 				.offset          = 0,
	// 			};

	// 			auto dst = SDL_GPUTextureRegion{
	// 				.texture = texture,
	// 			};

	// 			for (auto &&sub_image : img.sub_images)
	// 			{
	// 				src.offset = static_cast<uint32_t>(sub_image.offset);

	// 				dst.mip_level = sub_image.mipmap_index;
	// 				dst.layer     = sub_image.layer_index;
	// 				dst.w         = sub_image.width;
	// 				dst.h         = sub_image.height;
	// 				dst.d         = 1;

	// 				SDL_UploadToGPUTexture(copy_pass, &src, &dst, false);
	// 			}
	// 		}
	// 		SDL_EndGPUCopyPass(copy_pass);
	// 		SDL_SubmitGPUCommandBuffer(copy_cmd);
	// 	}
	// 	SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
	// }
}