import std;
import sdl;

auto main() -> int
{
	std::println("Current working dir: {}", std::filesystem::current_path().generic_string());

	auto result = SDL_Init(SDL_INIT_VIDEO);
	assert(result and "SDL could not be initialized.");

	auto wnd = SDL::make_window(800, 600, "SDL C++ Project Template.");
	auto gpu = SDL::make_gpu(SDL_GPU_SHADERFORMAT_SPIRV);

	result = SDL_ClaimWindowForGPUDevice(gpu.get(), wnd.get());
	assert(result and "Could not claim windows for gpu.");

	auto quit = false;
	auto evt  = SDL_Event{};
	while (not quit)
	{
		while (SDL_PollEvent(&evt))
		{
			switch (evt.type)
			{
			case SDL_EVENT_QUIT:
				quit = true;
				break;
			case SDL_EVENT_KEY_DOWN:
				// TODO: Handle Inputs
				quit = true;
				break;
			}
		}

		// TODO: State Updates
		// TODO: Do Rendering
	}

	SDL_ReleaseWindowFromGPUDevice(gpu.get(), wnd.get());

	SDL_Quit();
	return 0;
}
