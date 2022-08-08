#pragma once
#include <vector>
#include <cstring> // std::memcpy
#include <cassert>
#ifndef NDEBUG
#include <stack>
#endif
#include "StringType.hpp"
#include "StringifyNumber.hpp"

namespace August
{
	// The buffer ownership and character writing part of stringifying

	class Stringify
	{
		std::vector<Character> _buffer;
#ifndef NDEBUG
		std::stack<void*> _closing; // Validation of scope open/close order sequence
#endif
		std::size_t _closingCount = 0;
		StringifyNumber _numberToText;

		// We never copy or move
		void operator=(const Stringify&) = delete;
		void operator=(const Stringify&&) = delete;
		Stringify(const Stringify&) = delete;
		Stringify(const Stringify&&) = delete;

		Character* Extend(std::size_t by)
		{
			auto previousSize = _buffer.size() - _closingCount;
			_buffer.resize(previousSize + by + _closingCount);
			return _buffer.data() + previousSize;
		}

		Character* AppendOpen(std::size_t by)
		{
			auto previousSize = _buffer.size() - _closingCount;
			_closingCount++;
			_buffer.resize(previousSize + by + _closingCount);
			return _buffer.data() + previousSize;
		}

		void OnWriteNamedValue(bool& first, StringView name, StringView value)
		{
			auto target = Extend(name.length() + value.length() + (first ? 3 : 4)); // Quote,quote,colon + optional comma
			if (!first)
				*target++ = ',';
			*target++ = '"';
			std::memcpy(target, name.data(), name.length());
			target += name.length() + 2;
			target[-2] = '"';
			target[-1] = ':';
			std::memcpy(target, value.data(), value.length());
			first = false;
		}

		void OnWriteImmediateValue(bool& first, StringView value)
		{
			auto target = Extend(value.length() + (first ? 0 : 1)); // Optional comma
			if (!first)
				*target++ = ',';
			std::memcpy(target, value.data(), value.length());
			first = false;
		}

		static StringView GetNullText(bool first) noexcept
		{
			using namespace std::literals;
			static const StringView _nullText[] =
			{
				u8",null"sv,
				u8"null"sv,
			};
			return _nullText[static_cast<std::size_t>(first)];
		}
	public:
		Stringify() { }


		template<char c>
		void AppendClose([[maybe_unused]] void* test) noexcept
		{
			assert(_closingCount); // Closing too many?
			assert(_closing.top() == test); // Closing order differs to reverse opening order
			_buffer[_buffer.size() - _closingCount] = c;
			_closingCount--;
#ifndef NDEBUG
			_closing.pop();
#endif
		}

		template<char c>
		void AppendOpen(bool& first, [[maybe_unused]] void* test)
		{
			// As entry or value
			if (first)
				*AppendOpen(1) = c;
			else
			{
				static const char opening[] =
				{
					',',
					c
				};
				std::memcpy(AppendOpen(2), opening, 2);
			}
			first = false;
#ifndef NDEBUG
			_closing.push(test);
#endif
		}

		template<char c>
		void AppendOpen(bool& first, StringView name, [[maybe_unused]] void* test)
		{
			// As member of Object
			auto target = AppendOpen(name.length() + (first ? 4 : 5));
			if (!first)
				*target++ = ',';
			*target++ = '"';
			std::memcpy(target, name.data(), name.length());
			static const char opening[] =
			{
				'"',
				':',
				c
			};
			std::memcpy(target + name.length(), opening, 3);
			first = false;
#ifndef NDEBUG
			_closing.push(test);
#endif
		}


		template<typename ValueType_, typename std::enable_if<
			(std::numeric_limits<ValueType_>::is_integer && !std::is_same<ValueType_, bool>::value) || std::is_floating_point<ValueType_>::value,
			ValueType_>::type* = nullptr>
		void WriteValue(bool& first, ValueType_ value)
		{
			OnWriteImmediateValue(first, _numberToText.Stringify(value));
		}

		template<typename ValueType_, typename std::enable_if<
			(std::numeric_limits<ValueType_>::is_integer && !std::is_same<ValueType_, bool>::value) || std::is_floating_point<ValueType_>::value,
			ValueType_>::type* = nullptr>
		void WriteNamedValue(bool& first, StringView name, ValueType_ value)
		{
			OnWriteNamedValue(first, name, _numberToText.Stringify(value));
		}

		template<typename ValueType_, typename std::enable_if<std::is_same<ValueType_, bool>::value,
			ValueType_>::type* = nullptr>
		void WriteValue(bool& first, ValueType_ value)
		{
			using namespace std::literals;
			static const StringView _booleanText[] =
			{
				u8",false"sv,
				u8"false"sv,
				u8",true"sv,
				u8"true"sv,
			};
			auto text = _booleanText[static_cast<std::size_t>(first) | (static_cast<std::size_t>(value) << 1)];
			auto target = Extend(text.length());
			std::memcpy(target, text.data(), text.length());
			first = false;
		}

		template<typename ValueType_, typename std::enable_if<std::is_same<ValueType_, bool>::value,
			ValueType_>::type* = nullptr>
		void WriteNamedValue(bool& first, StringView name, ValueType_ value)
		{
			using namespace std::literals;
			OnWriteNamedValue(first, name, value ? u8"true"sv : u8"false"sv);
		}

		template<typename ValueType_, typename std::enable_if<std::is_same<ValueType_, std::nullptr_t>::value,
			ValueType_>::type* = nullptr>
		void WriteValue(bool& first, [[maybe_unused]] ValueType_ value)
		{
			OnWriteImmediateValue(first, GetNullText(first));
		}

		template<typename ValueType_, typename std::enable_if<std::is_same<ValueType_, std::nullptr_t>::value,
			ValueType_>::type* = nullptr>
		void WriteNamedValue(bool& first, StringView name, ValueType_ value)
		{
			OnWriteNamedValue(first, name, GetNullText(first));
		}

		template<typename ValueType_, typename std::enable_if<std::is_same<ValueType_, StringView>::value,
			ValueType_>::type* = nullptr>
		void WriteValue(bool& first, ValueType_ value)
		{
			auto target = Extend(value.length() + (first ? 2 : 3)); // Quote,quote + optional comma
			if (!first)
				*target++ = ',';
			*target++ = '"';
			std::memcpy(target, value.data(), value.length());
			target[value.length()] = '"';
			first = false;
		}

		template<typename ValueType_, typename std::enable_if<std::is_same<ValueType_, StringView>::value,
			ValueType_>::type* = nullptr>
		void WriteNamedValue(bool& first, StringView name, ValueType_ value)
		{
			auto target = Extend(name.length() + value.length() + (first ? 5 : 6)); // Quote*2,quote*2,colon + optional comma
			if (!first)
				*target++ = ',';
			*target++ = '"';
			std::memcpy(target, name.data(), name.length());
			target += name.length() + 3;
			target[-3] = '"';
			target[-2] = ':';
			target[-1] = '"';
			std::memcpy(target, value.data(), value.length());
			target[value.length()] = '"';
			first = false;
		}


		StringView ToStringView() const
		{
			assert(!_closingCount); // All StringifyArray & StringifyObject instances should be destructed before getting buffer results
			return StringView(_buffer.data(), _buffer.size());
		}
	};

}
