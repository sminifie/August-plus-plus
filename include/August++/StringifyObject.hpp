#pragma once
#ifndef NDEBUG
#include <set>
#include <cassert>
#endif
#include "Stringify.hpp"

namespace August
{
	class StringifyArray;
	class StringifyDocument;

	// A class to compose a JSON object over an instance lifetime

	class StringifyObject
	{
		friend class StringifyArray;
		friend class StringifyDocument;
		Stringify& _writer;
		bool _first = true;

		// We never copy or move
		void operator=(const StringifyObject&) = delete;
		void operator=(const StringifyObject&&) = delete;
		StringifyObject(const StringifyObject&) = delete;
		StringifyObject(const StringifyObject&&) = delete;

		StringifyObject(Stringify& writer, bool& first) : _writer(writer)
		{
			// Object as entry or value
			writer.AppendOpen<'{'>(first, this);
		}
		StringifyObject(Stringify& writer, bool& first, StringView name) : _writer(writer)
		{
			// Object as member of Object constructor
			writer.AppendOpen<'{'>(first, name, this);
		}

#ifndef NDEBUG
		std::set<Stringified> _keys;
		void ValidateKey(StringView name)
		{
			Stringified key(name);
			assert(_keys.find(key) == _keys.end()); // Key already exists
			_keys.insert(key);
		}
#endif
	public:
		~StringifyObject()
		{
			_writer.AppendClose<'}'>(this);
		}


		template<class ValueType_>
		void Write(StringView name, ValueType_ value)
		{
#ifndef NDEBUG
			ValidateKey(name);
#endif
			_writer.WriteNamedValue(_first, name, value);
		}

		StringifyObject& CreateObject(StringView name, std::function<void(StringifyObject&)>&& populate)
		{
#ifndef NDEBUG
			ValidateKey(name);
#endif
			StringifyObject subject(_writer, _first, name);
			populate(subject);
			return *this;
		}

		StringifyObject& CreateArray(StringView name, std::function<void(StringifyArray&)>&& populate);
	};

}
