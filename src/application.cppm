module;

export module application;

import std;
import sdl;
import clock;
import io;

namespace st = sdl::type;

export namespace project
{
	class application
	{
	public:
		// Rule of 5
		application(const application &)                     = default; // defaulted copy c'tor
		auto operator=(const application &) -> application & = default; // defaulted copy c'tor
		application(application &&)                          = default; // defaulted move c'tor
		auto operator=(application &&) -> application &      = default; // defaulted move c'tor

		// Public structures to start the application
		struct window_info
		{
			uint32_t width;
			uint32_t height;
			std::string_view title;
			SDL_WindowFlags flags = {};
		};

		struct gpu_info
		{
			SDL_GPUShaderFormat shader_format;
		};

		// Public API
		application(const window_info &wnd_info, const gpu_info &gpu_info)
		{
			wnd = sdl::make_window(wnd_info.width, wnd_info.height, wnd_info.title, wnd_info.flags);
			gpu = sdl::make_gpu(wnd.get(), gpu_info.shader_format);

			std::println("GPU Driver API: {}", SDL_GetGPUDeviceDriver(gpu.get()));
		}
		~application() = default;

		auto run() -> int
		{
			prepare_scene();

			clk.reset();
			while (not quit)
			{
				handle_sdl_events();

				update_state();

				draw();

				clk.tick();
			}

			std::println("Elapsed Time: {:.4f}s", clk.get_elapsed<clock::s>());

			return 0;
		}

	private:
		// functions to handle SDL Events and Inputs
		void handle_sdl_events();
		void handle_sdl_input();

		// Scene and Application State
		void prepare_scene();
		void update_state();

		// Show on screen
		void draw();

		// Structure to hold scene objects
		struct scene
		{
			SDL_FColor clear_color;
			st::gfx_pipeline_ptr basic_pipeline;
			st::gpu_buffer_ptr vertex_buffer;
			st::gpu_buffer_ptr index_buffer;
			uint32_t vertex_count;
			uint32_t index_count;
		};

		// Private members
		sdl::sdl_base sdl_o = {};      // SDL base object
		st::window_ptr wnd  = nullptr; // SDL window object
		st::gpu_ptr gpu     = nullptr; // SDL GPU object
		SDL_Event evt       = {};      // SDL Event object
		scene scn           = {};      // Project's Render context;

		clock clk = {};
		bool quit = false;
	};
}

using namespace project;
using namespace sdl;
using namespace sdl::type;

namespace
{
	struct vertex
	{
		glm::vec3 pos;
		glm::vec4 clr;
	};

	struct mesh
	{
		std::vector<vertex> vertices;
		std::vector<uint32_t> indices;
	};

	auto make_pipeline(SDL_GPUDevice *gpu, SDL_Window *wnd) -> gfx_pipeline_ptr
	{
		auto vs_shdr = shader_builder{
			.shader_binary = io::read_file("shaders/basic.vs_6_4.cso"),
			.stage         = shader_stage::vertex,
		};

		auto ps_shdr = shader_builder{
			.shader_binary = io::read_file("shaders/basic.ps_6_4.cso"),
			.stage         = shader_stage::fragment,
		};

		using VA = SDL_GPUVertexAttribute;
		auto va  = std::array{
            VA{
			   .location    = 0,
			   .buffer_slot = 0,
			   .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			   .offset      = 0,
            },
            VA{
			   .location    = 1,
			   .buffer_slot = 0,
			   .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
			   .offset      = sizeof(glm::vec3),
            },
		};

		using VBD = SDL_GPUVertexBufferDescription;
		auto vbd  = std::array{
            VBD{
			   .slot       = 0,
			   .pitch      = sizeof(vertex),
			   .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            },
		};

		auto pl = gfx_pipeline_builder{
			.vertex_shader              = vs_shdr.build(gpu),
			.fragment_shader            = ps_shdr.build(gpu),
			.vertex_attributes          = va,
			.vertex_buffer_descriptions = vbd,
			.color_format               = SDL_GetGPUSwapchainTextureFormat(gpu, wnd),
			.enable_depth_stencil       = false,
			.raster                     = raster_type::none_fill,
			.blend                      = blend_type::none,
			.topology                   = topology_type::triangle_list,
		};

		return pl.build(gpu);
	}

	auto make_square() -> mesh
	{
		constexpr auto x = 0.5f;
		constexpr auto y = 0.5f;

		return {
			.vertices = {
			  { { -x, +y, 0.f }, { 0.f, 0.f, 1.f, 1.f } },
			  { { +x, +y, 0.f }, { 1.f, 0.f, 0.f, 1.f } },
			  { { +x, -y, 0.f }, { 0.f, 1.f, 0.f, 1.f } },
			  { { -x, -y, 0.f }, { 0.f, 1.f, 1.f, 1.f } },
			},
			.indices = {
			  0, 1, 2, // face 1
			  2, 3, 0, // face 2
			},
		};
	}

	auto upload_mesh(SDL_GPUDevice *gpu, const mesh &msh) -> std::tuple<st::gpu_buffer_ptr, st::gpu_buffer_ptr>
	{
		auto vtx_bytes = io::as_byte_span(msh.vertices);
		auto idx_bytes = io::as_byte_span(msh.indices);

		auto vbo = make_gpu_buffer(gpu, SDL_GPU_BUFFERUSAGE_VERTEX, static_cast<uint32_t>(vtx_bytes.size()), "Vertex Buffer");
		upload_to_gpu(gpu, vbo.get(), vtx_bytes);

		auto ibo = make_gpu_buffer(gpu, SDL_GPU_BUFFERUSAGE_INDEX, static_cast<uint32_t>(idx_bytes.size()), "Index Buffer");
		upload_to_gpu(gpu, ibo.get(), idx_bytes);

		return {
			std::move(vbo),
			std::move(ibo)
		};
	}
}

void application::handle_sdl_events()
{
	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_EVENT_QUIT:
			quit = true;
			break;
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_MOUSE_MOTION:
		case SDL_EVENT_MOUSE_WHEEL:
			handle_sdl_input();
			break;
		}
	}
}

void application::handle_sdl_input()
{
	auto handle_keyboard = [&]([[maybe_unused]] const SDL_KeyboardEvent &key_evt) {
		quit = true;
	};

	auto handle_mouse_motion = [&]([[maybe_unused]] const SDL_MouseMotionEvent &mouse_evt) {

	};

	auto handle_mouse_wheel = [&]([[maybe_unused]] const SDL_MouseWheelEvent &wheel_evt) {

	};

	switch (evt.type)
	{
	case SDL_EVENT_KEY_DOWN:
		handle_keyboard(evt.key);
		break;
	case SDL_EVENT_MOUSE_MOTION:
		handle_mouse_motion(evt.motion);
		break;
	case SDL_EVENT_MOUSE_WHEEL:
		handle_mouse_wheel(evt.wheel);
		break;
	}
}

void application::prepare_scene()
{
	scn.clear_color    = { 0.2f, 0.2f, 0.4f, 1.0f };
	scn.basic_pipeline = make_pipeline(gpu.get(), wnd.get());

	auto sqr_msh     = make_square();
	scn.vertex_count = static_cast<uint32_t>(sqr_msh.vertices.size());
	scn.index_count  = static_cast<uint32_t>(sqr_msh.indices.size());

	std::tie(scn.vertex_buffer, scn.index_buffer) = upload_mesh(gpu.get(), sqr_msh);
}

void application::update_state()
{
}

void application::draw()
{
	auto cmd_buf = SDL_AcquireGPUCommandBuffer(gpu.get());
	assert(cmd_buf != nullptr and "Failed to acquire command buffer.");

	auto sc_img = sdl::next_swapchain_image(wnd.get(), cmd_buf);

	auto color_target = SDL_GPUColorTargetInfo{
		.texture     = sc_img,
		.clear_color = scn.clear_color,
		.load_op     = SDL_GPU_LOADOP_CLEAR,
		.store_op    = SDL_GPU_STOREOP_STORE,
	};

	auto render_pass = SDL_BeginGPURenderPass(cmd_buf, &color_target, 1, nullptr);
	{
		SDL_BindGPUGraphicsPipeline(render_pass, scn.basic_pipeline.get());

		auto vertex_bindings = std::array{
			SDL_GPUBufferBinding{
			  .buffer = scn.vertex_buffer.get(),
			  .offset = 0,
			},
		};
		SDL_BindGPUVertexBuffers(render_pass, 0, vertex_bindings.data(), static_cast<uint32_t>(vertex_bindings.size()));

		auto index_binding = SDL_GPUBufferBinding{
			.buffer = scn.index_buffer.get(),
			.offset = 0,
		};
		SDL_BindGPUIndexBuffer(render_pass, &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

		SDL_DrawGPUIndexedPrimitives(render_pass, scn.index_count, 1, 0, 0, 0);
	}
	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(cmd_buf);
}