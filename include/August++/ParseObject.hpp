#pragma once
#include <map>
#include <cassert>
#include "Parse.hpp"
#include "ParseToken.hpp"
#include "ParseString.hpp"
#include "ParseException.hpp"

namespace August
{

	// Providing parsing of an object
	// The rest of the time for querying, all functionality is in the base class std::map<StringView, ParseToken>

	class ParseObject : public std::map<StringView, ParseToken>
	{
		// We never instance, copy or move
		void operator=(const ParseObject&) = delete;
		void operator=(const ParseObject&&) = delete;
		ParseObject() = delete;
		ParseObject(const ParseObject&) = delete;
		ParseObject(const ParseObject&&) = delete;
	public:
		// The markers that are used by objects
		static constexpr Character OpeningBraces = '{';
		static constexpr Character ClosingBraces = '}';
		static constexpr Character NameSeparator = ':';
		static constexpr Character Comma = ',';

		// The first character matches '{', so interpret the rest as an object
		static ParseToken Parse(ObjectsTable& objects, ArraysTable& arrays, ParseIterator& start)
		{
			auto iterator = start;
			assert(*iterator == OpeningBraces);
			iterator++;
			auto character = SkipWhitespace(iterator);

			// Always reserve new object storage
			auto objectIndex = objects.size();
			objects.resize(objectIndex + 1);

			if (character == ClosingBraces)
				iterator++; // Empty object
			else
			{
				for (;;)
				{
					if (character != ParseString::Quotes)
						throw ParseException(iterator, "Expecting opening quotes for a member name");
					auto name = ParseString::ParseToView(iterator);
					character = SkipWhitespace(iterator);
					if (character != NameSeparator)
						throw ParseException(iterator, "Expecting ':' following object member name");
					iterator++;
					character = SkipWhitespace(iterator);
					auto token = ParseAny(objects, arrays, character, iterator);

					// Note that we don't keep a reference to &objects[objectIndex] for the duration
					// because ParseAny() may introduce more objects and resize and possibly move
					// the objects vector in memory. Therefore we dereference the index here every time.
					objects[objectIndex].emplace(name, token);

					character = SkipWhitespace(iterator);
					if (character == ClosingBraces)
					{
						iterator++;
						break;
					}
					if (character != Comma)
						throw ParseException(iterator, "Expecting ',' between object members");
					iterator++;
					character = SkipWhitespace(iterator);
				}
			}
			start = iterator;
			return ParseToken(&objects, objectIndex);
		}

		ParseToken const& operator[](StringView name) const
		{
			return at(name);
		}
	};

}
