
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
#include <August++/Document.hpp>
using namespace std;
using namespace August;

static auto GetPathToTestFiles(const char* executable)
{
	return filesystem::path(executable).parent_path() / ".." / "..";
}

static bool TestWithJsonCheckerFiles(const char* executable)
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
			Document document(content.data());
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

static bool TestWithNativeJsonFiles(const char* executable)
{
	cout << "Warning: These files are big! It takes a long long time for debug builds, only milliseconds for release." << endl;
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
			Document document(content.data());
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
static std::unique_ptr<Document> document;

static const Token& TestParse(StringView text)
{
	// Copy to a mutable buffer
	buffer.resize(text.size() + 1);
	std::copy(text.begin(), text.end(), buffer.begin());
	buffer[text.size()] = 0;

	document = std::make_unique<Document>(buffer.data());
	return *document;
}

static bool TestBasicTypes()
{
	auto success = true;

	// Integer
	if (TestParse(u8"456"sv).GetAs<int>() != 456)
		success = false;
	if (TestParse(u8"-456"sv).GetAs<int>() != -456)
		success = false;

	// Floating point
	if (TestParse(u8"456e-1"sv).GetAs<double>() != 45.6)
		success = false;
	if (TestParse(u8"-456e-1"sv).GetAs<double>() != -45.6)
		success = false;
	if (TestParse(u8"0.01"sv).GetAs<double>() != 0.01)
		success = false;
	if (TestParse(u8"-0.01"sv).GetAs<double>() != -0.01)
		success = false;
	if (TestParse(u8"0.01e-0"sv).GetAs<double>() != 0.01)
		success = false;
	if (TestParse(u8"-0.01e-0"sv).GetAs<double>() != -0.01)
		success = false;
	if (TestParse(u8"0.01e-1"sv).GetAs<double>() != 0.001)
		success = false;
	if (TestParse(u8"-0.01e-1"sv).GetAs<double>() != -0.001)
		success = false;
	if (TestParse(u8"456.78e-3"sv).GetAs<double>() != 0.45678)
		success = false;
	if (TestParse(u8"-456.78e-3"sv).GetAs<double>() != -0.45678)
		success = false;
	if (TestParse(u8"456.0000"sv).GetAs<double>() != 456.0)
		success = false;
	if (TestParse(u8"-456.0000"sv).GetAs<double>() != -456.0)
		success = false;
	if (TestParse(u8"134.46472199999994"sv).GetAs<double>() != 134.46472199999994)
		success = false;
	if (TestParse(u8"-134.46472199999994"sv).GetAs<double>() != -134.46472199999994)
		success = false;

	// Strings
	if (TestParse(u8"\"He\u0040llo\""sv).GetAs<StringView>() != u8"He@llo"sv)
		success = false;

	// Boolean
	if (!TestParse(u8"true"sv).GetAs<bool>())
		success = false;
	if (TestParse(u8"false"sv).GetAs<bool>())
		success = false;

	// Object
	auto& object = TestParse(u8"{\"Bool\":true,\"Object\":{\"Text\":\"Hello\"}}"sv).GetAs<Object>();
	if (!object.at(u8"Bool"sv).GetAs<bool>())
		success = false;
	if (object.at(u8"Object"sv).GetAs<Object>().at(u8"Text"sv).GetAs<String>() != u8"Hello"sv)
		success = false;

	// Array
	auto& array = TestParse(u8"[43,\"Text\",true,[\"Hello\"]]"sv).GetAs<Array>();
	if (array[0].GetAs<int>() != 43)
		success = false;
	if (array[1].GetAs<String>() != u8"Text"sv)
		success = false;
	if (!array[2].GetAs<bool>())
		success = false;
	if (array[3].GetAs<Array>()[0].GetAs<String>() != u8"Hello"sv)
		success = false;

	// Null
	if (!TestParse(u8"null"sv).IsNull())
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
		if (!TestWithJsonCheckerFiles(argv[0]) ||
			!TestWithNativeJsonFiles(argv[0]) ||
			!TestBasicTypes())
			exitCode = -1;
	}
	catch (const exception& error)
	{
		cout << error.what() << endl;
		exitCode = -2;
	}
	return exitCode;
}
