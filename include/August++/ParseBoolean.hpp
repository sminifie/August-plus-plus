#pragma once
#include <cassert>
#include "Parse.hpp"
#include "ParseToken.hpp"
#include "ParseException.hpp"

namespace August
{

	// Providing parsing of a boolean token

	class ParseBoolean
	{
		// We never instance, copy or move
		void operator=(const ParseBoolean&) = delete;
		void operator=(const ParseBoolean&&) = delete;
		ParseBoolean() = delete;
		ParseBoolean(const ParseBoolean&) = delete;
		ParseBoolean(const ParseBoolean&&) = delete;
	public:
		// The markers that indicate a boolean
		static constexpr auto FirstTrueCharacter = 't';
		static constexpr auto FirstFalseCharacter = 'f';

		// The first character matches 't' or 'f'. Does the rest match the expected "true" or "false"?
		static ParseToken ParseTrue(ParseIterator& start)
		{
			assert(*start == FirstTrueCharacter);
			if (start[1] == 'r' && start[2] == 'u' && start[3] == 'e')
			{
				start += 4;
				return ParseToken(true);
			}
			throw ParseException(start, "Expecting full boolean word 'true'");
		}
		static ParseToken ParseFalse(ParseIterator& start)
		{
			assert(*start == FirstFalseCharacter);
			if (start[1] == 'a' && start[2] == 'l' && start[3] == 's' && start[4] == 'e')
			{
				start += 5;
				return ParseToken(false);
			}
			throw ParseException(start, "Expecting full boolean word 'false'");
		}
	};

}
