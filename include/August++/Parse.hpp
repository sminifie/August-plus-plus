#pragma once
#include <map>
#include <vector>
#include <string_view>

namespace August
{

	// Fundamental types and utility functions for the parsing process

#if __cplusplus >= 202002L // C++20
	using Character = char8_t;
	using StringView = std::u8string_view;
#else
	using Character = char;
	using StringView = std::string_view;
#endif
	using ParseIterator = Character*;
	class Token;
	using ObjectsTable = std::vector<std::map<StringView, Token>>;
	using ArraysTable = std::vector<std::vector<Token>>;

	static constexpr bool IsWhitespace(Character byte) noexcept
	{
		// "Whitespace is any sequence of one or more of the following code points:
		// character tabulation (U+0009), line feed (U+000A), carriage return (U+000D), and space (U+0020)."
		return byte == 0x9 || byte == 0xA || byte == 0xD || byte == 0x20;
	}

	static constexpr Character SkipWhitespace(ParseIterator& start) noexcept
	{
		auto iterator = start;
		for (;;)
		{
			auto character = *iterator;
			if (!character)
			{
				start = iterator;
				return 0; // Return 0 to signal EOF whilst skipping whitespace
			}
			if (!IsWhitespace(character))
			{
				start = iterator;
				return character;
			}
			iterator++;
		}
	}

	// The main interpretation method given an initial non-whitespace character.
	// To avoid recursive includes, any parsing code can include this header and use this function but the definition is elsewhere
	extern Token ParseAny(ObjectsTable& objects, ArraysTable& arrays, Character character, ParseIterator& iterator);

}
