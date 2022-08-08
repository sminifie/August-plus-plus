#pragma once
#include <cassert>
#include "Parse.hpp"
#include "ParseToken.hpp"
#include "ParseException.hpp"

namespace August
{

	// Providing parsing of a null token

	class ParseNull
	{
		// We never instance, copy or move
		void operator=(const ParseNull&) = delete;
		void operator=(const ParseNull&&) = delete;
		ParseNull() = delete;
		ParseNull(const ParseNull&) = delete;
		ParseNull(const ParseNull&&) = delete;
	public:
		// The marker that indicates a null
		static constexpr auto FirstCharacter = 'n';

		// The first character matches 'n'. Does the rest match the expected "null"?
		static ParseToken Parse(ParseIterator& start)
		{
			assert(*start == FirstCharacter);
			if (start[1] == 'u' && start[2] == 'l' && start[3] == 'l')
			{
				start += 4;
				return ParseToken();
			}
			throw ParseException(start, "Expecting full 'null' word");
		}
	};

}
