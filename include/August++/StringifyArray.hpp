#pragma once
#include "Stringify.hpp"

namespace August
{
	class StringifyObject;
	class StringifyDocument;

	// A class to compose a JSON array over an instance lifetime

	class StringifyArray
	{
		friend class StringifyObject;
		friend class StringifyDocument;
		Stringify& _writer;
		bool _first = true;

		// We never copy or move
		void operator=(const StringifyArray&) = delete;
		void operator=(const StringifyArray&&) = delete;
		StringifyArray(const StringifyArray&) = delete;
		StringifyArray(const StringifyArray&&) = delete;

		StringifyArray(Stringify& writer, bool& first) : _writer(writer)
		{
			// Array as entry or value
			writer.AppendOpen<'['>(first, this);
		}
		StringifyArray(Stringify& writer, bool& first, StringView name) : _writer(writer)
		{
			// Array as member of Object constructor
			writer.AppendOpen<'['>(first, name, this);
		}
	public:
		~StringifyArray()
		{
			_writer.AppendClose<']'>(this);
		}


		template<class ValueType_>
		void Write(ValueType_ value)
		{
			_writer.WriteValue(_first, value);
		}

		StringifyArray& CreateObject(std::function<void(StringifyObject&)>&& populate);

		StringifyArray& CreateArray(std::function<void(StringifyArray&)>&& populate)
		{
			StringifyArray subject(_writer, _first);
			populate(subject);
			return *this;
		}
	};

}
