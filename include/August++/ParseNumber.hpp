#pragma once
#include <cassert>
#include "Parse.hpp"
#include "ParseToken.hpp"
#include "ParseException.hpp"
#include "PowerOf10.hpp"

namespace August
{

	// Providing parsing of a number. Numbers can be integer or floating point
	// depending upon the presence of a decimal point or exponent.
	// The rest of the time for querying, all functionality is in the base class std::map<StringView, ParseToken>

	class ParseNumber
	{
		// We never instance, copy or move
		void operator=(const ParseNumber&) = delete;
		void operator=(const ParseNumber&&) = delete;
		ParseNumber() = delete;
		ParseNumber(const ParseNumber&) = delete;
		ParseNumber(const ParseNumber&&) = delete;

		static constexpr Character IsExponentMarker(Character character) noexcept
		{
			return (character | 0x20) == 'e';
		}
		static constexpr Character IsDigit(Character character) noexcept
		{
			return character >= '0' && character <= '9';
		}

	public:
		// The first character hasn't matched any of the other easily identified symbols or letters,
		// so we can only assume it's a number or an unrecognised token.
		static ParseToken Parse(Character character, ParseIterator& start)
		{
			// number = [ minus ] int [ frac ] [ exp ]
			// e = %x65 / %x45; e E
			// exp = e[minus / plus] 1 * DIGIT
			// frac = decimal-point 1 * DIGIT
			// note: int cannot have leading zeros
			auto iterator = start;

			// Leading minus sign
			assert(character && character == *iterator);
			auto negative = (character == '-');
			if (negative)
			{
				iterator++;
				if (!*iterator)
				{
					// Minus sign then EOF
					throw ParseException(iterator, "Numerical digits expected");
				}
				character = *iterator;
			}

			// ParseNumber digit must follow
			assert(character && character == *iterator);
			if (!IsDigit(character))
			{
				if (!negative)
				{
					// Complain` when no minus sign then no numerical digit
					throw ParseException(start, "Unrecognised token");
				}
				// We've seen a minus sign but no digit followed
				throw ParseException(iterator, "ParseNumber expected");
			}

			// Decimal digit sequence starting to form an integer
			assert(*iterator && character == *iterator);
			std::int64_t integerValue = 0;
			if (character != '0')
			{
				// Non-zero removes the possibility of single zero or zero prefixed
				for (;;)
				{
					integerValue = integerValue * 10 + static_cast<std::int64_t>(character) - static_cast<std::int64_t>('0');
					iterator++;
					if (!*iterator)
					{
						// Just an integer then EOF
						start = iterator;
						return ParseToken(negative ? -integerValue : integerValue);
					}
					character = *iterator;
					if (!IsDigit(character))
					{
						if (character == '.' || IsExponentMarker(character))
						{
							// Digits are followed by a dot or exponent
							// This is the only continuation exiting the for() loop
							break;
						}
						// We've seen an optional minus sign and at least one digit but attached trailing is something else
						start = iterator;
						return ParseToken(negative ? -integerValue : integerValue);
					}
				}
			}
			else
			{
				iterator++;
				if (!*iterator || (!IsDigit(*iterator) && *iterator != '.' && !IsExponentMarker(*iterator)))
				{
					// Just a (possibly negative) zero then EOF/non-number
					start = iterator;
					return ParseToken(int64_t(0));
				}
				character = *iterator;
				if (IsDigit(character))
				{
					// Zero followed by anything other than dot or exponent is not allowed
					throw ParseException(iterator, "Zero prefix not allowed");
				}
			}

			// Decimal point followed by a fraction number part
			assert(character && character == *iterator);
			int fractionalExponentValue;
			auto floatingPointValue = static_cast<double>(integerValue);
			if (character == '.')
			{
				iterator++;
				if (!*iterator || !IsDigit(*iterator))
				{
					// ParseNumber and decimal point followed by EOF or non-digit
					throw ParseException(iterator, "Expecting number to follow decimal point");
				}
				auto fractionalDigitsStart = iterator;
				character = *iterator;
				for (;;)
				{
					floatingPointValue = floatingPointValue * 10.0 + static_cast<double>(character) - static_cast<std::int64_t>('0');
					iterator++;
					if (!*iterator)
					{
						// Digits past the decimal point and EOF
						start = iterator;
						return ParseToken((negative ? -floatingPointValue : floatingPointValue) / PowerOf10<double>::For(static_cast<std::size_t>(iterator - fractionalDigitsStart)));
					}
					character = *iterator;
					if (!IsDigit(character))
					{
						if (IsExponentMarker(character))
						{
							// The only loop exit to continue to exponent
							break;
						}
						// Decimal number followed by unexpected character
						start = iterator;
						return ParseToken((negative ? -floatingPointValue : floatingPointValue) / PowerOf10<double>::For(static_cast<std::size_t>(iterator - fractionalDigitsStart)));
					}
				}
				fractionalExponentValue = static_cast<int>(fractionalDigitsStart - iterator);
			}
			else
				fractionalExponentValue = 0;

			// Can only be exponent digits
			assert(*iterator && character == *iterator);
			iterator++;
			if (!*iterator || (!IsDigit(*iterator) && *iterator != '-' && *iterator != '+'))
			{
				// Exponent marker followed by EOF or non-digit
				throw ParseException(iterator, "Expecting number for exponent");
			}
			character = *iterator;
			auto negativeExponent = (character == '-');
			if (negativeExponent || character == '+')
			{
				iterator++;
				if (!*iterator || !IsDigit(*iterator))
				{
					// Exponent marker followed by +/- then EOF or non-digit
					throw ParseException(iterator, "Expecting number for exponent");
				}
				character = *iterator;
			}
			assert(*iterator && character == *iterator && IsDigit(character));
			int exponentValue = 0;
			for (;;)
			{
				exponentValue = exponentValue * 10 + (character - '0');
				iterator++;
				if (!*iterator || !IsDigit(*iterator))
				{
					// Exponent and EOF or unexpected
					start = iterator;
					exponentValue = (negativeExponent ? -exponentValue : exponentValue) + fractionalExponentValue;
					if (exponentValue < 0)
						floatingPointValue = floatingPointValue / PowerOf10<double>::For(static_cast<std::size_t>(-exponentValue));
					else
						floatingPointValue = floatingPointValue * PowerOf10<double>::For(static_cast<std::size_t>(exponentValue));
					return ParseToken(negative ? -floatingPointValue : floatingPointValue);
				}
				character = *iterator;
			}
		}
	};

}
