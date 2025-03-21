module;

export module clock;

import std;

export namespace project
{
	class clock
	{
	public:
		using hrc = std::chrono::high_resolution_clock;
		using ns  = std::chrono::duration<double, std::nano>;
		using us  = std::chrono::duration<double, std::micro>;
		using ms  = std::chrono::duration<double, std::milli>;
		using s   = std::chrono::duration<double, std::ratio<1>>;

	public:
		void tick()
		{
			auto current_time = hrc::now();
			delta_time        = current_time - previous_time;
			previous_time     = current_time;

			elapsed_time += delta_time;
		}

		void reset()
		{
			previous_time = hrc::now();
			delta_time    = {};
			elapsed_time  = {};
		}

		template <typename duration_type>
		[[nodiscard]] auto get_delta() const -> double
		{
			return std::chrono::duration_cast<duration_type>(delta_time).count();
		}

		template <typename duration_type>
		[[nodiscard]] auto get_elapsed() const -> double
		{
			return std::chrono::duration_cast<duration_type>(elapsed_time).count();
		}

	private:
		hrc::time_point previous_time = hrc::now();

		hrc::duration delta_time{};
		hrc::duration elapsed_time{};
	};
}