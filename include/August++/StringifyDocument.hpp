#pragma once
#include <stdexcept> // std::runtime_error
#include <functional>
#include "StringType.hpp"
#include "StringifyObject.hpp"
#include "StringifyArray.hpp"

namespace August
{

	// The main class for creating JSON.

	class StringifyDocument
	{
		Stringify _writer;
		bool _first = true;

		// We never copy or move
		void operator=(const StringifyDocument&) = delete;
		void operator=(const StringifyDocument&&) = delete;
		StringifyDocument(const StringifyDocument&) = delete;
		StringifyDocument(const StringifyDocument&&) = delete;

		void CheckForExistingRoot() const
		{
			if (!_first)
				throw std::runtime_error("There is already a root element");
		}
	public:
		StringifyDocument()
		{
		}


		StringView Stringify() const
		{
			return _writer.ToStringView();
		}


		template<class ValueType_>
		void Write(ValueType_ value)
		{
			CheckForExistingRoot();
			_writer.WriteValue(_first, value);
		}

		StringifyDocument& CreateObject(std::function<void(StringifyObject&)>&& populate)
		{
			CheckForExistingRoot();
			StringifyObject subject(_writer, _first);
			populate(subject);
			return *this;
		}

		StringifyDocument& CreateArray(std::function<void(StringifyArray&)>&& populate)
		{
			CheckForExistingRoot();
			StringifyArray subject(_writer, _first);
			populate(subject);
			return *this;
		}
	};

	StringifyObject& StringifyObject::CreateArray(StringView name, std::function<void(StringifyArray&)>&& populate)
	{
#ifndef NDEBUG
		ValidateKey(name);
#endif
		StringifyArray subject(_writer, _first, name);
		populate(subject);
		return *this;
	}

	StringifyArray& StringifyArray::CreateObject(std::function<void(StringifyObject&)>&& populate)
	{
		StringifyObject subject(_writer, _first);
		populate(subject);
		return *this;
	}

}
