module;

export module application;

import std;
import sdl;

namespace st = sdl::type;

export namespace project
{
	class application
	{
	public:
		application(const application &)                     = default; // defaulted copy c'tor
		auto operator=(const application &) -> application & = default; // defaulted copy c'tor
		application(application &&)                          = default; // defaulted move c'tor
		auto operator=(application &&) -> application &      = default; // defaulted move c'tor

	public:
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

	public:
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

			while (not quit)
			{
				handle_sdl_events();

				update_state();

				draw();
			}
			return 0;
		}

	private:
		void handle_sdl_events();
		void handle_sdl_input();

		void prepare_scene();
		void update_state();
		void draw();

	private:
		st::gpu_ptr gpu    = nullptr;
		st::window_ptr wnd = nullptr;

		bool quit     = false;
		SDL_Event evt = {};
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
}

void application::update_state()
{
}

void application::draw()
{
}