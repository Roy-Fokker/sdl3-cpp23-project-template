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
			auto result = SDL_Init(SDL_INIT_VIDEO);
			assert(result and "SDL could not be initialized.");

			wnd = sdl::make_window(wnd_info.width, wnd_info.height, wnd_info.title, wnd_info.flags);
			gpu = sdl::make_gpu(wnd.get(), gpu_info.shader_format);

			std::println("GPU Driver API: {}", SDL_GetGPUDeviceDriver(gpu.get()));
		}
		~application()
		{

			gpu = {};
			wnd = {};

			SDL_Quit();
		}

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

			scn = {};

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
		};

		// Private members
		st::gpu_ptr gpu    = nullptr;
		st::window_ptr wnd = nullptr;

		clock clk = {};

		bool quit     = false;
		SDL_Event evt = {};

		scene scn = {};
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
			.enable_depth_stencil       = true,
			.raster                     = raster_type::none_fill,
			.blend                      = blend_type::none,
			.topology                   = topology_type::triangle_list,
		};

		return pl.build(gpu);
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
	scn.clear_color = { 0.2f, 0.2f, 0.4f, 1.0f };

	scn.basic_pipeline = make_pipeline(gpu.get(), wnd.get());
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
	}
	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(cmd_buf);
}