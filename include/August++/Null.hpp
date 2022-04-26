#pragma once
#include <cassert>
#include "Parse.hpp"
#include "Token.hpp"
#include "ParseException.hpp"

namespace August
{

	// Providing parsing of a null token

	class Null
	{
		// We never instance, copy or move
		void operator=(const Null&) = delete;
		void operator=(const Null&&) = delete;
		Null() = delete;
		Null(const Null&) = delete;
		Null(const Null&&) = delete;
	public:
		// The marker that indicates a null
		static constexpr auto FirstCharacter = 'n';

		// The first character matches 'n'. Does the rest match the expected "null"?
		static Token Parse(ParseIterator& start)
		{
			assert(*start == FirstCharacter);
			if (start[1] == 'u' && start[2] == 'l' && start[3] == 'l')
			{
				start += 4;
				return Token();
			}
			throw ParseException(start, "Expecting full 'null' word");
		}
	};

}
