
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

#include <fstream>
#include <iostream>
#include <string_view>
#include <filesystem>
#include <chrono>
#include <August++/ParseDocument.hpp>
#include <August++/StringifyDocument.hpp>
using namespace std;
using namespace August;

static auto GetPathToTestFiles(const char* executable)
{
	return filesystem::path(executable).parent_path() / ".." / "..";
}

static bool TestReadingWithJsonCheckerFiles(const char* executable)
{
	auto success = true;
	auto testFolder = GetPathToTestFiles(executable) / "JSON_checker" / "test-files";
	for (const auto& entry : filesystem::directory_iterator(testFolder))
	{
		auto expectFailure = entry.path().filename().u8string().substr(0, 4) == u8"fail"sv;

		auto size = std::filesystem::file_size(entry);
		std::vector<Character> content(static_cast<std::size_t>(size) + 1);
		std::ifstream file(entry.path().string(), std::ios::in | std::ios::binary);
		file.read(reinterpret_cast<char*>(content.data()), static_cast<std::streamsize>(size));
		file.close();
		content[static_cast<std::size_t>(size)] = 0;

		bool didFail;
		try
		{
			ParseDocument parseDocument(content.data());
			didFail = false;
		}
		catch (const ParseException& error)
		{
			auto lineAndIndex = ParseException::DocumentLineAndIndex(content.data(), static_cast<std::size_t>(size), error.Where);
			cout << entry.path().string() << "(" << lineAndIndex.first << "," << lineAndIndex.second << "): " << error.what() << endl;
			didFail = true;
		}
		catch (const exception& error)
		{
			cout << entry.path().string() << ": " << error.what() << endl;
			didFail = true;
		}
		if (expectFailure != didFail)
		{
			cout << "** Incorrect " << (expectFailure ? "should fail" : "should pass") << " **" << endl;
			success = false;
		}
		else
			cout << "Correct " << (expectFailure ? "did fail" : "did pass") << endl;
	}
	return success;
}

static bool TestReadingWithNativeJsonFiles(const char* executable)
{
	cout << "Warning: These files are big! It takes a long long time for debug builds (maybe 20 minutes), only milliseconds for release." << endl;
	auto success = true;
	auto testFolder = GetPathToTestFiles(executable) / "nativejson-benchmark";
	for (const auto& entry : filesystem::directory_iterator(testFolder))
	{
		auto size = std::filesystem::file_size(entry);
		std::vector<Character> content(static_cast<std::size_t>(size) + 1);
		std::ifstream file(entry.path().string(), std::ios::in | std::ios::binary);
		file.read(reinterpret_cast<char*>(content.data()), static_cast<std::streamsize>(size));
		file.close();
		content[static_cast<std::size_t>(size)] = 0;

		auto start = std::chrono::high_resolution_clock::now();
		cout << entry.path().string() << "..." << endl;
		try
		{
			ParseDocument parseDocument(content.data());
		}
		catch (const exception&)
		{
			success = false;
		}
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		cout << duration << "ms complete." << endl;
	}
	return success;
}

static std::vector<Character> buffer;
static std::unique_ptr<ParseDocument> parseDocument;

static const ParseToken& TestParseValue(StringView text)
{
	// Copy to a mutable buffer
	buffer.resize(text.size() + 1);
	std::copy(text.begin(), text.end(), buffer.begin());
	buffer[text.size()] = 0;

	parseDocument = std::make_unique<ParseDocument>(buffer.data());
	return *parseDocument;
}

static bool TestReadingBasicTypes()
{
	auto success = true;

	// Integer
	if (TestParseValue(u8"456"sv).As<int>() != 456)
		success = false;
	if (TestParseValue(u8"-456"sv).As<int>() != -456)
		success = false;
	if (TestParseValue(u8"456"sv).As<unsigned>() != 456)
		success = false;

	// Floating point
	if (TestParseValue(u8"456e-1"sv).As<double>() != 45.6)
		success = false;
	if (TestParseValue(u8"-456e-1"sv).As<double>() != -45.6)
		success = false;
	if (TestParseValue(u8"456e-1"sv).As<float>() != 45.6f)
		success = false;
	if (TestParseValue(u8"0.01"sv).As<double>() != 0.01)
		success = false;
	if (TestParseValue(u8"-0.01"sv).As<double>() != -0.01)
		success = false;
	if (TestParseValue(u8"0.01e-0"sv).As<double>() != 0.01)
		success = false;
	if (TestParseValue(u8"-0.01e-0"sv).As<double>() != -0.01)
		success = false;
	if (TestParseValue(u8"0.01e-1"sv).As<double>() != 0.001)
		success = false;
	if (TestParseValue(u8"-0.01e-1"sv).As<double>() != -0.001)
		success = false;
	if (TestParseValue(u8"456.78e-3"sv).As<double>() != 0.45678)
		success = false;
	if (TestParseValue(u8"-456.78e-3"sv).As<double>() != -0.45678)
		success = false;
	if (TestParseValue(u8"456.0000"sv).As<double>() != 456.0)
		success = false;
	if (TestParseValue(u8"-456.0000"sv).As<double>() != -456.0)
		success = false;
	if (TestParseValue(u8"134.46472199999994"sv).As<double>() != 134.46472199999994)
		success = false;
	if (TestParseValue(u8"-134.46472199999994"sv).As<double>() != -134.46472199999994)
		success = false;

	// Strings
	if (TestParseValue(u8"\"He\u0040llo\""sv).As<StringView>() != u8"He@llo"sv)
		success = false;

	// ParseBoolean
	if (!TestParseValue(u8"true"sv).As<bool>())
		success = false;
	if (TestParseValue(u8"false"sv).As<bool>())
		success = false;

	// ParseObject
	auto& object = TestParseValue(u8"{\"Bool\":true,\"ParseObject\":{\"Text\":\"Hello\"}}"sv).As<ParseObject>();
	if (!object[u8"Bool"sv].As<bool>())
		success = false;
	if (object[u8"ParseObject"sv].As<ParseObject>()[u8"Text"sv].As<ParseString>() != u8"Hello"sv)
		success = false;

	// ParseArray
	auto& array = TestParseValue(u8"[43,\"Text\",true,[\"Hello\"]]"sv).As<ParseArray>();
	if (array[0].As<int>() != 43)
		success = false;
	if (array[1].As<ParseString>() != u8"Text"sv)
		success = false;
	if (!array[2].As<bool>())
		success = false;
	if (array[3].As<ParseArray>()[0].As<ParseString>() != u8"Hello"sv)
		success = false;

	// ParseNull
	if (!TestParseValue(u8"null"sv).IsNull())
		success = false;
	return success;
}

template<class ValueType_>
static Stringified TestStringifyValue(ValueType_ value)
{
	StringifyDocument stringifyDocument;
	stringifyDocument.Write(value);
	return Stringified(stringifyDocument.Stringify());
}

static bool TestStringifyBasicTypes()
{
	auto success = true;

	// Integer
	if (TestStringifyValue(0) != u8"0"sv)
		success = false;
	if (TestStringifyValue(0u) != u8"0"sv)
		success = false;
	if (TestStringifyValue(46) != u8"46"sv)
		success = false;
	if (TestStringifyValue(-46) != u8"-46"sv)
		success = false;
	if (TestStringifyValue(12345678910) != u8"12345678910"sv)
		success = false;

	// Floating point
	if (TestStringifyValue(45.6) != u8"45.6"sv)
		success = false;
	if (TestStringifyValue(-45.6) != u8"-45.6"sv)
		success = false;
	if (TestStringifyValue(45.6f) != u8"45.6"sv)
		success = false;
	if (TestStringifyValue(134.46472199999994) != u8"134.465"sv) // Truncated?
		success = false;
	if (TestStringifyValue(-134.46472199999994) != u8"-134.465"sv) // Truncated?
		success = false;

	// Strings
	if (TestStringifyValue(u8"He\u0040llo"sv) != u8"\"He@llo\""sv)
		success = false;

	// Boolean
	if (TestStringifyValue(true) != u8"true"sv)
		success = false;
	if (TestStringifyValue(false) != u8"false"sv)
		success = false;

	// Null
	if (TestStringifyValue(nullptr) != u8"null"sv)
		success = false;

	// Object
	if (StringifyDocument().CreateObject([](StringifyObject& obj1)
		{
			obj1.Write(u8"signed"sv, -46);
			obj1.Write(u8"unsigned"sv, 46u);
			obj1.Write(u8"float"sv, 47.2);
			obj1.Write(u8"string"sv, u8"Hello"sv);
			obj1.Write(u8"boolean"sv, true);
			obj1.CreateObject(u8"object"sv, [](StringifyObject&) {});
			obj1.CreateArray(u8"array"sv, [](StringifyArray&) {});
		}).Stringify() != u8"{\"signed\":-46,\"unsigned\":46,\"float\":47.2,\"string\":\"Hello\",\"boolean\":true,\"object\":{},\"array\":[]}")
		success = false;

	// Array
	if (StringifyDocument().CreateArray([](StringifyArray& arr1)
		{
			arr1.Write(-46);
			arr1.Write(46u);
			arr1.Write(47.2);
			arr1.Write(u8"Hello"sv);
			arr1.Write(true);
			arr1.CreateObject([](StringifyObject&) {});
			arr1.CreateArray([](StringifyArray&) {});
		}).Stringify() != u8"[-46,46,47.2,\"Hello\",true,{},[]]")
		success = false;

	return success;
}

int main(int argc, char* argv[])
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
#endif
	auto exitCode = 0;
	try
	{
		if (!TestStringifyBasicTypes())
			exitCode = -1;

		if (!TestReadingBasicTypes() ||
			!TestReadingWithJsonCheckerFiles(argv[0]) ||
			!TestReadingWithNativeJsonFiles(argv[0]))
			exitCode = -1;
	}
	catch (const exception& error)
	{
		cout << error.what() << endl;
		exitCode = -2;
	}
	return exitCode;
}
