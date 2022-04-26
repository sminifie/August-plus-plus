#pragma once
#include <memory>
#include "Token.hpp"
#include "Array.hpp"
#include "Object.hpp"
#include "String.hpp"
#include "Boolean.hpp"
#include "Null.hpp"
#include "Number.hpp"
#include "Parse.hpp"
#include "ParseException.hpp"

namespace August
{

	// The main class for parsing JSON.
	// Parsing happens during construct from a buffer which the caller provides.
	// The provided buffer needs to be kept alive as long as the Document class is in use.
	// The provided buffer also needs to be zero terminated.

	class Document : public Token
	{
		// We never copy or move
		void operator=(const Document&) = delete;
		void operator=(const Document&&) = delete;
		Document(const Document&) = delete;
		Document(const Document&&) = delete;

		// The odds of having more than a couple of arrays or objects is greater than none,
		// so we start off with non-empty to avoid the first resize once any are added
		static constexpr std::size_t DefaultTableSize = 4;

		ObjectsTable _objects;
		ArraysTable _arrays;
	public:
		// The JSON source must be mutable and zero terminated
		// Note that this constructor doesn't persist the buffer, and it needs to live as long as this instance
		explicit Document(ParseIterator iterator)
		{
			_objects.reserve(DefaultTableSize);
			_arrays.reserve(DefaultTableSize);
			auto character = SkipWhitespace(iterator);
			static_cast<Token&>(*this) = ParseAny(_objects, _arrays, character, iterator);
			character = SkipWhitespace(iterator);
			if (character)
				throw ParseException(iterator, "Unexpected content after main document");
		}
	};

	Token ParseAny(ObjectsTable& objects, ArraysTable& arrays, Character character, ParseIterator& iterator)
	{
		switch (character)
		{
		case Null::FirstCharacter:
			return Null::Parse(iterator);

		case Object::OpeningBraces:
			return Object::Parse(objects, arrays, iterator);

		case Array::OpeningBracket:
			return Array::Parse(objects, arrays, iterator);

		case String::Quotes:
			return String::Parse(iterator);

		case Boolean::FirstTrueCharacter:
			return Boolean::ParseTrue(iterator);

		case Boolean::FirstFalseCharacter:
			return Boolean::ParseFalse(iterator);

		default:
			return Number::Parse(character, iterator);
		}
	}

}
