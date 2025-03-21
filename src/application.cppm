module;

export module application;

import std;
import sdl;
import clock;

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
			gpu = sdl::make_gpu(gpu_info.shader_format);

			std::println("GPU Driver API: {}", SDL_GetGPUDeviceDriver(gpu.get()));

			result = SDL_ClaimWindowForGPUDevice(gpu.get(), wnd.get());
			assert(result and "Could not claim windows for gpu.");
		}
		~application()
		{
			SDL_ReleaseWindowFromGPUDevice(gpu.get(), wnd.get());

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
	auto handle_keyboard = [&](const SDL_KeyboardEvent &key_evt) {
		quit = true;
	};

	auto handle_mouse_motion = [&](const SDL_MouseMotionEvent &mouse_evt) {

	};

	auto handle_mouse_wheel = [&](const SDL_MouseWheelEvent &wheel_evt) {

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