#pragma once
#include <cassert>
#include "Parse.hpp"
#include "Token.hpp"
#include "ParseException.hpp"

namespace August
{

	// Providing parsing of an array
	// The rest of the time for querying, all functionality is in the base class std::vector<Token>

	class Array : public std::vector<Token>
	{
		// We never instance, copy or move
		void operator=(const Array&) = delete;
		void operator=(const Array&&) = delete;
		Array() = delete;
		Array(const Array&) = delete;
		Array(const Array&&) = delete;

		// The odds of arrays having more than a couple of entries is greater than none,
		// so we start off with non-empty to avoid the first resize once any are added
		static constexpr std::size_t DefaultSize = 2;
	public:
		// The markers that are used by arrays
		static constexpr Character OpeningBracket = '[';
		static constexpr Character ClosingBracket = ']';
		static constexpr Character CommaSeparator = ',';

		// The first character matches '[', so interpret the rest as an array
		static Token Parse(ObjectsTable& objects, ArraysTable& arrays, ParseIterator& start)
		{
			auto iterator = start;
			assert(*iterator == OpeningBracket);
			auto character = SkipWhitespace(++iterator);

			// Always reserve a new array
			auto arrayIndex = arrays.size();
			arrays.emplace_back().reserve(DefaultSize);

			if (character == ClosingBracket)
				iterator++; // Empty array
			else
			{
				for (;;)
				{
					auto token = ParseAny(objects, arrays, character, iterator);

					// Note that we don't keep a reference to &arrays[arrayIndex] for the duration
					// because ParseAny() may introduce more arrays and resize and possibly move
					// the arrays vector in memory. Therefore we dereference the index here every time.
					arrays[arrayIndex].emplace_back(token);

					character = SkipWhitespace(iterator);
					if (character == ClosingBracket)
					{
						iterator++;
						break;
					}
					if (character != CommaSeparator)
						throw ParseException(iterator, "Expecting comma separating array elements or closing bracket");
					iterator++;
					character = SkipWhitespace(iterator);
				}
			}
			start = iterator;
			return Token(&arrays, arrayIndex);
		}
	};

}
