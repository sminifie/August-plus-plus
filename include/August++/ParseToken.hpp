#pragma once
#include <string_view>
#include <typeinfo> // bad_cast
#include <map>
#include <vector>
#include <type_traits>
#include "Parse.hpp"

namespace August
{
	class ParseArray;
	class ParseObject;
	class ParseString;

	// A token representing one of the several types.
	// The type of any token can only be tested at runtime.

	class ParseToken
	{
		enum class TokenType
		{
			ParseNull,
			ParseObject,
			ParseArray,
			ParseString,
			BooleanTrue,
			BooleanFalse,
			Integer,
			FloatingPoint
		};

		union FloatingPointIntegerOrPointer
		{
			constexpr explicit FloatingPointIntegerOrPointer(double floatingPoint) noexcept : _floatingPoint(floatingPoint) {}
			constexpr explicit FloatingPointIntegerOrPointer(std::int64_t integer) noexcept : _integer(integer) {}
			constexpr explicit FloatingPointIntegerOrPointer(const void* pointer) noexcept : _pointer(pointer) {}

			double _floatingPoint;
			std::int64_t _integer;
			const void* _pointer;
		};
		FloatingPointIntegerOrPointer _data;
		std::size_t _index; // ParseObject index, array index or string length combined with token type
		constexpr TokenType GetType() const noexcept
		{
			return static_cast<TokenType>(_index & 0x7);
		}
		constexpr std::size_t GetIndex() const noexcept
		{
			return _index >> 3;
		}
		constexpr std::size_t MakeIndex(TokenType type) const noexcept
		{
			return static_cast<std::size_t>(type);
		}
		constexpr std::size_t MakeIndex(TokenType type, std::size_t index) const noexcept
		{
			return (index << 3) | MakeIndex(type);
		}
	public:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 26495) // Uninitialised members is okay - they're never used.
#endif
#if __cplusplus >= 202002L // C++20
		constexpr // C++20 feature for constexpr and uninitialised members
#endif
		ParseToken() noexcept : 
			_data(nullptr),
			_index(MakeIndex(TokenType::ParseNull))
		{
		}
		explicit constexpr ParseToken(bool boolean) noexcept :
			_data(nullptr),
			_index(MakeIndex(boolean ? TokenType::BooleanTrue : TokenType::BooleanFalse))
		{
		}
		explicit constexpr ParseToken(double floatingPoint) noexcept : 
			_data(floatingPoint),
			_index(MakeIndex(TokenType::FloatingPoint))
		{
		}
		explicit constexpr ParseToken(std::int64_t integer) noexcept :
			_data(integer),
			_index(MakeIndex(TokenType::Integer))
		{
		}
		constexpr ParseToken(const Character* string, std::size_t stringLength) noexcept :
			_data(static_cast<const void*>(string)),
			_index(MakeIndex(TokenType::ParseString, stringLength))
		{
		}
		constexpr ParseToken(const ArraysTable* arrays, std::size_t index) noexcept : 
			_data(static_cast<const void*>(arrays)),
			_index(MakeIndex(TokenType::ParseArray, index))
		{
		}
		constexpr ParseToken(const ObjectsTable* objects, std::size_t index) noexcept : 
			_data(static_cast<const void*>(objects)),
			_index(MakeIndex(TokenType::ParseObject, index))
		{
		}
		explicit constexpr ParseToken(const ParseToken& rhs) noexcept : 
			_data(rhs._data),
			_index(rhs._index)
		{
		}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

		ParseToken& operator=(const ParseToken& rhs) noexcept
		{
			_data = rhs._data;
			_index = rhs._index;
			return *this;
		}
		ParseToken& operator=(const ParseToken&& rhs) noexcept
		{
			_data = rhs._data;
			_index = rhs._index;
			return *this;
		}


		bool IsNull() const noexcept
		{
			return GetType() == TokenType::ParseNull;
		}

		bool IsObject() const noexcept
		{
			return GetType() == TokenType::ParseObject;
		}

		bool IsArray() const noexcept
		{
			return GetType() == TokenType::ParseArray;
		}

		bool IsBoolean() const noexcept
		{
			return GetType() == TokenType::BooleanTrue || GetType() == TokenType::BooleanFalse;
		}

		bool IsInteger() const noexcept
		{
			return GetType() == TokenType::Integer;
		}

		bool IsFloatingPoint() const noexcept
		{
			return GetType() == TokenType::FloatingPoint;
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::numeric_limits<ValueType_>::is_integer && 
			!std::is_same<ValueType_, bool>::value,
			ValueType_>::type As() const
		{
			auto type = GetType();
			if (type != TokenType::Integer)
			{
				// If the file provides a floating point number, and you're expecting an integer, 
				// that's an error in the same way as other incorrect type
				throw std::bad_cast();
			}
			return static_cast<ValueType_>(_data._integer);
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::is_floating_point<ValueType_>::value,
			ValueType_>::type As() const
		{
			auto type = GetType();
			if (type == TokenType::FloatingPoint)
				return static_cast<ValueType_>(_data._floatingPoint);
			if (type == TokenType::Integer)
			{
				// Integer can be promoted to floating point
				return static_cast<ValueType_>(_data._integer);
			}
			throw std::bad_cast();
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::is_same<ValueType_, bool>::value,
			ValueType_>::type As() const
		{
			auto type = GetType();
			if (type == TokenType::BooleanTrue)
				return true;
			if (type == TokenType::BooleanFalse)
				return false;
			throw std::bad_cast();
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::is_same<ValueType_, ParseString>::value ||
			std::is_same<ValueType_, StringView>::value,
			ValueType_>::type As() const
		{
			auto type = GetType();
			if (type != TokenType::ParseString)
				throw std::bad_cast();
			return ValueType_(static_cast<const Character*>(_data._pointer), GetIndex());
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::is_same<ValueType_, ParseArray>::value ||
			std::is_same<ValueType_, std::vector<ParseToken>>::value,
			const ValueType_&>::type As() const
		{
			auto type = GetType();
			if (type != TokenType::ParseArray)
				throw std::bad_cast();
			return static_cast<const ValueType_&>((*static_cast<const ArraysTable*>(_data._pointer))[GetIndex()]);
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::is_same<ValueType_, ParseObject>::value ||
			std::is_same<ValueType_, std::map<StringView, ParseToken>>::value,
			const ValueType_&>::type As() const
		{
			auto type = GetType();
			if (type != TokenType::ParseObject)
				throw std::bad_cast();
			return static_cast<const ValueType_&>((*static_cast<const ObjectsTable*>(_data._pointer))[GetIndex()]);
		}
	};

}
