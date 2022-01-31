#pragma once

#include <fstream>
#include <stdarg.h>

#include <string>
using std::string;

class Log{
public:
	Log() {}
	~Log() {}

	static bool Init(string sName = "applog.txt", bool echoConsole = true);
	static void Write(const char *msg, ...);
	static std::ofstream& Stream();
	static void EchoConsoleOn();
	static void EchoConsoleOff();

protected:
	static bool m_bEchoConsole;
	static std::ofstream m_appLog;
};

