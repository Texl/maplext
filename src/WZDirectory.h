#pragma once

#include "Globals.h"

#include "WZFileEntry.h"

struct WZDirectory{
	WZDirectory() {}
	WZDirectory(string name, int size, int checksum, int offset) : m_sName(name), m_iSize(size), m_iChecksum(checksum), m_iOffset(offset) {}
	~WZDirectory() {}

	string	m_sName;
	int		m_iSize;
	int		m_iChecksum;
	int		m_iOffset;
	int		m_iRealOffset;

	vector<WZFileEntry> m_vFiles;
	vector<WZDirectory> m_vSubdirectories;

	WZDirectory*	GetDirectoryByName(string sName);
	WZFileEntry*	GetFileByName(string sName);
};
