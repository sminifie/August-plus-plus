#pragma once
#include <type_traits> // make_unsigned
#include <cmath>
#include <cstdio> // snprintf
#include "StringType.hpp"

namespace August
{

	// Truning integer and floating point values to strings

	class StringifyNumber
	{
		Character _numberScratchBuffer[308 + 2] = { 0 }; // maximum digits in 64bit double plus sign, plus zero terminator

		// TODO: Influences using reverse technique plus others from https://github.com/fmtlib/fmt/blob/master/include/fmt/format.h

		// Converts value in the range [0, 100] to a string.
		// Note GCC generates slightly better code when value is pointer-size.
		constexpr const char* Get2Digits(std::size_t value) {
			return &
				"0001020304050607080910111213141516171819"
				"2021222324252627282930313233343536373839"
				"4041424344454647484950515253545556575859"
				"6061626364656667686970717273747576777879"
				"8081828384858687888990919293949596979899"[value * 2];
		}

		template<class ValueType_>
		Character* OnStringifyUnsigned(Character* target, ValueType_ value)
		{
			// Two digits at a time to half the number of divides
			for (;;)
			{
				target -= 2;
				auto modulus = value % ValueType_(100);
				auto twoDigits = Get2Digits(static_cast<std::size_t>(modulus));
				std::memcpy(target, twoDigits, 2);
				value /= ValueType_(100);
				if (!value)
					return modulus < 10 ? target + 1 : target;
			}
		}
	public:
		template<class ValueType_, typename std::enable_if<std::numeric_limits<ValueType_>::is_integer&&
			std::is_unsigned<ValueType_>::value, ValueType_>::type* = nullptr>
		StringView Stringify(ValueType_ value)
		{
			auto end = &_numberScratchBuffer[sizeof _numberScratchBuffer];
			auto target = OnStringifyUnsigned(end, value);
			return StringView(target, static_cast<StringView::size_type>(end - target));
		}

		template<class ValueType_, typename std::enable_if<std::numeric_limits<ValueType_>::is_integer&&
			std::is_signed<ValueType_>::value, ValueType_>::type* = nullptr>
		StringView Stringify(ValueType_ value)
		{
			if (value >= 0)
				return Stringify(static_cast<typename std::make_unsigned<ValueType_>::type>(value));

			auto end = &_numberScratchBuffer[sizeof _numberScratchBuffer];
			auto target = OnStringifyUnsigned(end, static_cast<typename std::make_unsigned<ValueType_>::type>(-value));
			*--target = '-';
			return StringView(target, static_cast<StringView::size_type>(end - target));
		}


		template<class ValueType_, typename std::enable_if<std::is_floating_point<ValueType_>::value, ValueType_>::type* = nullptr>
		StringView Stringify(ValueType_ value)
		{
			// TODO: Is this really the fastest implementation?
			// TODO: Also, it's a shame that we've gotten away without using the standard library in anger until this function :(
			// We're using %G to be the most abbreviated form available in expectation that a major aim is compact JSON.
			auto length = snprintf(reinterpret_cast<char*>(&_numberScratchBuffer[0]), sizeof _numberScratchBuffer, "%G", value);
			return StringView(_numberScratchBuffer, static_cast<StringView::size_type>(length));
		}
	};

}
