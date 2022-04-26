#pragma once
#include <stdexcept>
#include "Parse.hpp"

namespace August
{

	// The location and description for any parsing error.
	// Use the DocumentLineAndIndex() function to establish a more human-useful location in the document.

	class ParseException : public std::runtime_error
	{
		void operator=(const ParseException&) = delete;
		void operator=(const ParseException&&) = delete;
		ParseException(const ParseException&) = delete;
		ParseException() = delete;
	public:
		template<class IteratorType_>
		ParseException(IteratorType_ where, const char* message)
			: std::runtime_error(message), Where(&*where)
		{
		}

		using DocumentLocation = const Character*;
		const DocumentLocation Where;

		// Count the line and character position from a pointer into the middle of the document
		static std::pair<std::size_t, std::size_t> DocumentLineAndIndex(ParseIterator document, std::size_t size, DocumentLocation where)
		{
			if (where < document || where > document + size)
				return std::pair<std::size_t, std::size_t>(0u, 0u);
			std::size_t line = 1;
			std::size_t index = 1;
			while (document != where)
			{
				auto character = *document++;
				if (character == '\r')
				{
					if (*document == '\n')
						document++;
					line++;
					index = 1;
				}
				else if (character == '\n')
				{
					line++;
					index = 1;
				}
				else
					index++;
			}
			return std::pair<std::size_t, std::size_t>(line, index);
		}
	};

}
