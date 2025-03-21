module;

export module io;

import std;

export namespace io
{
	// Convience alias for a span of bytes
	using byte_array = std::vector<std::byte>;
	using byte_span  = std::span<const std::byte>;
	using byte_spans = std::span<byte_span>;

	auto as_byte_span(const auto &src) -> byte_span
	{
		return std::span{
			reinterpret_cast<const std::byte *>(&src),
			sizeof(src)
		};
	}

	auto as_byte_span(const std::ranges::contiguous_range auto &src) -> byte_span
	{
		auto src_span   = std::span{ src };      // convert to a span,
		auto byte_size  = src_span.size_bytes(); // so we can get size_bytes
		auto byte_start = reinterpret_cast<const std::byte *>(src.data());
		return { byte_start, byte_size };
	}

	auto offset_ptr(void *ptr, std::ptrdiff_t offset) -> void *
	{
		return reinterpret_cast<std::byte *>(ptr) + offset;
	}

	auto read_file(const std::filesystem::path &filename) -> byte_array
	{
		std::println("Reading file: {}", filename.string());

		auto file = std::ifstream(filename, std::ios::in | std::ios::binary);

		assert(file.good() and "failed to open file!");

		auto file_size = std::filesystem::file_size(filename);
		auto buffer    = byte_array(file_size);

		file.read(reinterpret_cast<char *>(buffer.data()), file_size);

		file.close();

		return buffer;
	}
}