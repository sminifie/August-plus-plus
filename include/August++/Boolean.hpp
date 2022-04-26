#pragma once
#include <cassert>
#include "Parse.hpp"
#include "Token.hpp"
#include "ParseException.hpp"

namespace August
{

	// Providing parsing of a boolean token

	class Boolean
	{
		// We never instance, copy or move
		void operator=(const Boolean&) = delete;
		void operator=(const Boolean&&) = delete;
		Boolean() = delete;
		Boolean(const Boolean&) = delete;
		Boolean(const Boolean&&) = delete;
	public:
		// The markers that indicate a boolean
		static constexpr auto FirstTrueCharacter = 't';
		static constexpr auto FirstFalseCharacter = 'f';

		// The first character matches 't' or 'f'. Does the rest match the expected "true" or "false"?
		static Token ParseTrue(ParseIterator& start)
		{
			assert(*start == FirstTrueCharacter);
			if (start[1] == 'r' && start[2] == 'u' && start[3] == 'e')
			{
				start += 4;
				return Token(true);
			}
			throw ParseException(start, "Expecting full boolean word 'true'");
		}
		static Token ParseFalse(ParseIterator& start)
		{
			assert(*start == FirstFalseCharacter);
			if (start[1] == 'a' && start[2] == 'l' && start[3] == 's' && start[4] == 'e')
			{
				start += 5;
				return Token(false);
			}
			throw ParseException(start, "Expecting full boolean word 'false'");
		}
	};

}
