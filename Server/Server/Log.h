#pragma once
#pragma warning(disable:4996)
#include <fstream>
#include <mutex>

#define log_assert(expression) (void)(																						\
            (!!(expression)) || _assertion_log_write(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)) ||	\
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0)								\
        )

static mutex writeFileMutex;
const int _maxLogBufferLength = 512;

template<typename ... Arguments>
void swprintfLog(char* buffer, const char* const format, Arguments... args)
{
	SYSTEMTIME tmSystem;
	::GetLocalTime(&tmSystem);

	// Print the current time 
	const int length = sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02d  ",
		tmSystem.wYear, tmSystem.wMonth, tmSystem.wDay, tmSystem.wHour, tmSystem.wMinute, tmSystem.wSecond);

	sprintf(buffer + length, format, args...);
}

template<typename ... Arguments>
void swprintfLog(wchar_t* buffer, const wchar_t* format, Arguments... args)
{
	SYSTEMTIME tmSystem;
	::GetLocalTime(&tmSystem);

	// Print the current time 
	const int length = swprintf(buffer, L"%04d-%02d-%02dT%02d:%02d:%02d  ",
		tmSystem.wYear, tmSystem.wMonth, tmSystem.wDay, tmSystem.wHour, tmSystem.wMinute, tmSystem.wSecond);

	swprintf(buffer + length, format, args...);
}

template<typename ... Arguments>
void LogWrite(const wchar_t* format, Arguments... args)
{
	wchar_t str[_maxLogBufferLength];
	swprintfLog(str, format, args...);
	str[_maxLogBufferLength - 1] = L'\0';

	wofstream fout;
	{
		lock_guard<mutex> lg(writeFileMutex);
		fout.open("ServerLog.txt", std::ios_base::out | std::ios_base::app);
		fout << str << endl;
		fout.close();
	}
}

template<typename ... Arguments>
void LogWrite(const char* format, Arguments... args)
{
	char str[_maxLogBufferLength];
	swprintfLog(str, format, args...);
	str[_maxLogBufferLength - 1] = '\0';

	ofstream fout;
	{
		lock_guard<mutex> lg(writeFileMutex);
		fout.open("ServerLog.txt", std::ios_base::out | std::ios_base::app);
		fout << str << endl;
		fout.close();
	}
}

inline bool _assertion_log_write(const wchar_t* message, const wchar_t* str2, unsigned int line)
{
	LogWrite(L"[에러] Assertion failed: %s, %s, line %d", message, str2, line);
	return false;
}

template<typename ... Arguments>
void LogPrintf(const wchar_t* format, Arguments... args)
{
	wchar_t str[_maxLogBufferLength];
	swprintfLog(str, format, args...);
	str[_maxLogBufferLength - 1] = L'\0';

	wcout << str << endl;
}

template<typename ... Arguments>
void LogPrintf(const char* format, Arguments... args)
{
	char str[_maxLogBufferLength];
	swprintfLog(str, format, args...);
	str[_maxLogBufferLength - 1] = L'\0';

	cout << str << endl;
}

template<typename ... Arguments>
void Log(const wchar_t* format, Arguments... args)
{
	wchar_t str[_maxLogBufferLength];
	swprintfLog(str, format, args...);
	str[_maxLogBufferLength - 1] = L'\0';

	wofstream fout;
	{
		lock_guard<mutex> lg(writeFileMutex);
		fout.open("ServerLog.txt", std::ios_base::out | std::ios_base::app);
		fout << str << endl;
		fout.close();
	}

	wcout << str << endl;
}

template<typename ... Arguments>
void Log(const char* format, Arguments... args)
{
	char str[_maxLogBufferLength];
	swprintfLog(str, format, args...);
	str[_maxLogBufferLength - 1] = '\0';

	ofstream fout;
	{
		lock_guard<mutex> lg(writeFileMutex);
		fout.open("ServerLog.txt", std::ios_base::out | std::ios_base::app);
		fout << str << endl;
		fout.close();
	}

	cout << str << endl;
}



