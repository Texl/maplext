#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <vector>
using std::vector;

#include <cstring> //for stricmp
#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

//stricmp for STL strings
inline bool stricmp(const std::string &s1, const std::string &s2)
{
	return stricmp(s1.c_str(), s2.c_str()) == 0;
}
 
#include "Log.h"
#include "Profiler.h"
#include "Tokenizer.h"
