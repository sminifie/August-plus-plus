#pragma once
#include <string>
#include <string_view>

namespace August
{

	// Fundamental string types

#if __cplusplus >= 202002L // C++20
	using Character = char8_t;
	using StringView = std::u8string_view;
	using Stringified = std::u8string;
#else
	using Character = char;
	using StringView = std::string_view;
	using Stringified = std::string;
#endif

}
