#pragma once
#include <memory>
#include <cassert>
#include <algorithm>
#include "Parse.hpp"
#include "Token.hpp"
#include "ParseException.hpp"

namespace August
{

	// Providing parsing of a string to a string_view, including transformation of explicit code points
	// The rest of the time for querying, all functionality is in the base class std::u8string_view

	class String : public StringView
	{
		// We never copy or move
		void operator=(const String&) = delete;
		void operator=(const String&&) = delete;
		String() = delete;

		static bool FindNextEscape(ParseIterator& at)
		{
			ParseIterator test = at;
			for (;;)
			{
				switch (*test)
				{
				case 0:
					throw ParseException(at, "No end of string");

				case '"':
					at = test;
					return false;

				case '\\':
					at = test;
					return true;

				case '\t':
				case '\r':
				case '\n':
					throw ParseException(at, "Unsupported character in string");
				}
				test++;
			}
		}

		static std::uint16_t HexDigit(ParseIterator at)
		{
			auto c = *at;
			if (c >= '0' && c <= '9')
				return static_cast<std::uint16_t>(c - '0');
			c |= 0x20;
			if (c >= 'a' && c <= 'f')
				return static_cast<std::uint16_t>(c - 'a' + 0x10);
			throw ParseException(at, "Bad hex digit");
		}

		static constexpr ParseIterator WriteUtf8(ParseIterator at, std::uint16_t code) noexcept
		{
			if (!(code & ~std::uint16_t(0x7f))) // 7 bits
				*at++ = static_cast<Character>(code & 0x7f);
			if (!(code & ~std::uint16_t(0x7ff))) // 11 bits
			{
				*at++ = static_cast<Character>(((code & 0x7c0) >> 6) | 0xc0);
				*at++ = static_cast<Character>((code & 0x03f) | 0x80);
			}
			else // 16 bits
			{
				*at++ = static_cast<Character>(((code & 0xf000) >> 12) | 0xe0);
				*at++ = static_cast<Character>(((code & 0x0fc0) >> 6) | 0x80);
				*at++ = static_cast<Character>((code & 0x003f) | 0x80);
			}
			return at;
		}

	public:
		constexpr String(const String& rhs) noexcept : StringView(rhs)
		{
		}
		constexpr String(const StringView::const_pointer base, const StringView::size_type count) noexcept : StringView(base, count)
		{
		}

		// The marker that indicate a boolean
		static constexpr auto Quotes = '"';

		static Token Parse(ParseIterator& start)
		{
			auto stringView = ParseToView(start);
			return Token(stringView.data(), stringView.size());
		}

		static StringView ParseToView(ParseIterator& start)
		{
			assert(*start == Quotes);
			start++;

			// string = quotation-mark *char quotation-mark
			auto iterator = start;
			auto translatedEnd = iterator;
			for (;;)
			{
				auto segmentStart = iterator;
				auto escapeFound = FindNextEscape(iterator);
				if (translatedEnd != segmentStart) // If we have previous conversions
				{
					if (segmentStart != iterator) // If we covered any non-escape chars in the scan
					{
						// The segment just scanned needs copying back a few bytes to 
						// close the gap between (the larger) original ascii and the unprocessed section.
						translatedEnd = std::copy(segmentStart, iterator, translatedEnd);
					}
				}
				else
					translatedEnd = iterator;
				if (!escapeFound)
				{
					iterator++;
					auto beginning = start;
					start = iterator;
					return StringView(reinterpret_cast<StringView::const_pointer>(beginning), static_cast<std::size_t>(translatedEnd - beginning));
				}
				iterator++;
				if (!*iterator)
					throw ParseException(iterator, "EOF during escape sequence");
				auto adjustedCode = *iterator;
				switch (adjustedCode)
				{
				case '"':
				case '\\':
				case '/':
					// %x22 /          ; "    quotation mark  U+0022
					// %x5C /          ; \    reverse solidus U+005C
					// %x2F /          ; /    solidus         U+002F
					break;

				case 'b':
					// %x62 /          ; b    backspace       U+0008
					adjustedCode = 0x8;
					break;

				case 'f':
					// %x66 /          ; f    form feed       U+000C
					adjustedCode = 0xc;
					break;

				case 'n':
					// %x6E /          ; n    line feed       U+000A
					adjustedCode = 0xa;
					break;

				case 'r':
					// %x72 /          ; r    carriage return U+000D
					adjustedCode = 0xd;
					break;

				case 't':
					// %x74 /          ; t    tab             U+0009
					adjustedCode = 0x9;
					break;

				case 'u':
					// %x75 4HEXDIG )  ; uXXXX                U+XXXX
				{
					// Because ascii "/uXXXX" is always more bytes (6) than the equivalent utf-8 (max. 3 bytes),
					// we can in-place encode and overwrite, at the penalty of shuffling following characters in blocks.
					auto shift = 16u;
					std::uint16_t code = 0;
					do
					{
						iterator++;
						if (!*iterator)
							throw ParseException(iterator, "Bad escape");
						shift -= 4;
						code |= HexDigit(iterator) << shift;
					} while (shift);
					iterator++;
					translatedEnd = WriteUtf8(translatedEnd, code);
				}
				continue;

				default:
					throw ParseException(iterator, "Unrecognised escape sequence");
				}
				*translatedEnd++ = adjustedCode;
				iterator++;
			}
		}
	};

}
