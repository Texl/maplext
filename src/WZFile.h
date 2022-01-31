#pragma once

#include "Globals.h"

#include "IMGFile.h"
#include "WZFileEntry.h"
#include "WZDirectory.h"

class WZFile{
public:
	WZFile();
	WZFile(string filename);
	~WZFile();

	bool Open(string filename, string filter = "");
	void Close();

	void ListDirectory(WZDirectory *pDirectory = NULL,  string path = "");
	void ExportXML(FILE* fileOut, WZDirectory *pDirectory = NULL,  string path = "", int depth = 0);
	void ExtractImages(WZDirectory *pDirectory = NULL,  string path = "");
	void ExtractSounds(WZDirectory *pDirectory = NULL,  string path = "");
	void ExtractFiles(WZDirectory *pDirectory = NULL,  string path = "");

	string DecodeStringAtOffset(int offset);

private:
	int FindVersion();
	void ParseDirectory(WZDirectory *pDirectory);
	void FindOffsets(WZDirectory *pDirectory, int &iStartOffset);
	void ReadFiles(WZDirectory *pDirectory, string path = "");

	string GetString(int length = 0);
	string DecodeString();
	unsigned char GetByte();
	int GetInt();
	int GetValue();

	FILE*				m_fileIn;
	int					m_iFileSize;
	int					m_iHeaderSize;
	int					m_iVersion;
	string				m_sFilename;
	string				m_sPKG;
	string				m_sCopyright;
	WZDirectory			m_dirRoot;

	string				m_sFilter;
};
