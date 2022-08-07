#pragma once
#include <memory>
#include "ParseToken.hpp"
#include "ParseArray.hpp"
#include "ParseObject.hpp"
#include "ParseString.hpp"
#include "ParseBoolean.hpp"
#include "ParseNull.hpp"
#include "ParseNumber.hpp"
#include "Parse.hpp"
#include "ParseException.hpp"

namespace August
{

	// The main class for parsing JSON.
	// Parsing happens during construct from a buffer which the caller provides.
	// The provided buffer needs to be kept alive as long as the ParseDocument class is in use.
	// The provided buffer also needs to be zero terminated.

	class ParseDocument : public ParseToken
	{
		// We never copy or move
		void operator=(const ParseDocument&) = delete;
		void operator=(const ParseDocument&&) = delete;
		ParseDocument(const ParseDocument&) = delete;
		ParseDocument(const ParseDocument&&) = delete;

		// The odds of having more than a couple of arrays or objects is greater than none,
		// so we start off with non-empty to avoid the first resize once any are added
		static constexpr std::size_t DefaultTableSize = 4;

		ObjectsTable _objects;
		ArraysTable _arrays;
	public:
		// The JSON source must be mutable and zero terminated
		// Note that this constructor doesn't persist the buffer, and it needs to live as long as this instance
		explicit ParseDocument(ParseIterator iterator)
		{
			_objects.reserve(DefaultTableSize);
			_arrays.reserve(DefaultTableSize);
			auto character = SkipWhitespace(iterator);
			static_cast<ParseToken&>(*this) = ParseAny(_objects, _arrays, character, iterator);
			character = SkipWhitespace(iterator);
			if (character)
				throw ParseException(iterator, "Unexpected content after main document");
		}
	};

	inline ParseToken ParseAny(ObjectsTable& objects, ArraysTable& arrays, Character character, ParseIterator& iterator)
	{
		switch (character)
		{
		case ParseNull::FirstCharacter:
			return ParseNull::Parse(iterator);

		case ParseObject::OpeningBraces:
			return ParseObject::Parse(objects, arrays, iterator);

		case ParseArray::OpeningBracket:
			return ParseArray::Parse(objects, arrays, iterator);

		case ParseString::Quotes:
			return ParseString::Parse(iterator);

		case ParseBoolean::FirstTrueCharacter:
			return ParseBoolean::ParseTrue(iterator);

		case ParseBoolean::FirstFalseCharacter:
			return ParseBoolean::ParseFalse(iterator);

		default:
			return ParseNumber::Parse(character, iterator);
		}
	}

}
