#pragma once
#include <map>
#include <vector>
#include <string_view>
#include "StringType.hpp"

namespace August
{

	// Fundamental types and utility functions for the parsing process

	using ParseIterator = Character*;
	class ParseToken;
	using ObjectsTable = std::vector<std::map<StringView, ParseToken>>;
	using ArraysTable = std::vector<std::vector<ParseToken>>;

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
	extern ParseToken ParseAny(ObjectsTable& objects, ArraysTable& arrays, Character character, ParseIterator& iterator);

}
