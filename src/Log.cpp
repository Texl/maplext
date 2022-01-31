#include "Log.h"

bool Log::Init(string sName, bool echoConsole)
{
	m_bEchoConsole = echoConsole;
	m_appLog.open(sName.c_str());
	if(!m_appLog.is_open())
		return false;
	return true;
}

void Log::Write(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	char szBuf[1024];
	vsprintf(szBuf, msg, args);

	m_appLog << szBuf;
	if(m_bEchoConsole)
		printf("%s", szBuf);
	m_appLog.flush();
}

std::ofstream& Log::Stream()
{
	return m_appLog;
}

void Log::EchoConsoleOn()
{
	m_bEchoConsole = true;
}

void Log::EchoConsoleOff()
{
	m_bEchoConsole = false;
}

std::ofstream Log::m_appLog;
bool Log::m_bEchoConsole = false;