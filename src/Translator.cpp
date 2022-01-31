//Alan Horne 2006
/*
#include <iostream>
#include "WZParser.h"

using namespace std;

WZParser g_parser;

int main(int argc, char **argv)
{
	Log::Init("strings.txt", true);

	string fileName;

	//get filename
	if(argc == 2)
	{
		fileName = argv[1];
		Log::Write("scanning %s...\n", argv[1]);
	}
	else
	{
		fileName = "Data.wz";
		Log::Write("no input file specified.\nscanning data.wz...\n");
	}
	
	g_parser.Open(fileName);

	int offset;

	cout << "offset: ";
	cin >> offset;

	while(offset >= 0)
	{
		string str = g_parser.DecodeStringAtOffset(offset);
		Log::Write("%08x: %s\n", offset, str.c_str());
		cout << "offset: ";
		cin >> offset;
	}

	g_parser.Close();
	return 0;
}
*/