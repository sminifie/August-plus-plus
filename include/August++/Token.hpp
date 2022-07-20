#pragma once
#include <string_view>
#include <typeinfo> // bad_cast
#include <map>
#include <vector>
#include <type_traits>
#include "Parse.hpp"

namespace August
{
	class Array;
	class Object;
	class String;

	// A token representing one of the several types.
	// The type of any token can only be tested at runtime.

	class Token
	{
		enum class TokenType
		{
			Null,
			Object,
			Array,
			String,
			BooleanTrue,
			BooleanFalse,
			Integer,
			FloatingPoint
		};

		union FloatingPointIntegerOrPointer
		{
			double _floatingPoint;
			std::int64_t _integer;
			const void* _pointer;
		};
		FloatingPointIntegerOrPointer _data;
		std::size_t _index; // Object index, array index or string length combined with token type
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
		Token() noexcept
		{
			_index = MakeIndex(TokenType::Null);
		}
		explicit constexpr Token(bool boolean) noexcept : _index(MakeIndex(boolean ? TokenType::BooleanTrue : TokenType::BooleanFalse))
		{
		}
		explicit constexpr Token(double floatingPoint) noexcept : _index(MakeIndex(TokenType::FloatingPoint))
		{
			_data._floatingPoint = floatingPoint;
		}
		explicit constexpr Token(std::int64_t integer) noexcept : _index(MakeIndex(TokenType::Integer))
		{
			_data._integer = integer;
		}
		constexpr Token(const Character* string, std::size_t stringLength) noexcept : _index(MakeIndex(TokenType::String, stringLength))
		{
			_data._pointer = static_cast<const void*>(string);
		}
		constexpr Token(const ArraysTable* arrays, std::size_t index) noexcept : _index(MakeIndex(TokenType::Array, index))
		{
			_data._pointer = static_cast<const void*>(arrays);
		}
		constexpr Token(const ObjectsTable* objects, std::size_t index) noexcept : _index(MakeIndex(TokenType::Object, index))
		{
			_data._pointer = static_cast<const void*>(objects);
		}
		explicit constexpr Token(const Token& rhs) noexcept : _index(rhs._index)
		{
			_data = rhs._data;
		}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

		Token& operator=(const Token& rhs) noexcept
		{
			_data = rhs._data;
			_index = rhs._index;
			return *this;
		}
		Token& operator=(const Token&& rhs) noexcept
		{
			_data = rhs._data;
			_index = rhs._index;
			return *this;
		}


		bool IsNull() const noexcept
		{
			return GetType() == TokenType::Null;
		}

		bool IsObject() const noexcept
		{
			return GetType() == TokenType::Object;
		}

		bool IsArray() const noexcept
		{
			return GetType() == TokenType::Array;
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
			ValueType_>::type GetAs() const
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
			ValueType_>::type GetAs() const
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
			ValueType_>::type GetAs() const
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
			std::is_same<ValueType_, String>::value ||
			std::is_same<ValueType_, StringView>::value,
			ValueType_>::type GetAs() const
		{
			auto type = GetType();
			if (type != TokenType::String)
				throw std::bad_cast();
			return ValueType_(static_cast<const Character*>(_data._pointer), GetIndex());
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::is_same<ValueType_, Array>::value ||
			std::is_same<ValueType_, std::vector<Token>>::value,
			const ValueType_&>::type GetAs() const
		{
			auto type = GetType();
			if (type != TokenType::Array)
				throw std::bad_cast();
			return static_cast<const ValueType_&>((*static_cast<const ArraysTable*>(_data._pointer))[GetIndex()]);
		}

		template<typename ValueType_>
		typename std::enable_if<
			std::is_same<ValueType_, Object>::value ||
			std::is_same<ValueType_, std::map<StringView, Token>>::value,
			const ValueType_&>::type GetAs() const
		{
			auto type = GetType();
			if (type != TokenType::Object)
				throw std::bad_cast();
			return static_cast<const ValueType_&>((*static_cast<const ObjectsTable*>(_data._pointer))[GetIndex()]);
		}
	};

}
