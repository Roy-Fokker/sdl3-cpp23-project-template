import std;
import application;

auto main() -> int
{
	std::println("Current working dir: {}", std::filesystem::current_path().generic_string());

	auto app = project::application({ 800, 600, "SDL3 C++ 23 Project Template" }, { SDL_GPU_SHADERFORMAT_SPIRV });

	return app.run();
}
