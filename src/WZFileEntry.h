#pragma once

#include "Globals.h"

#include "IMGFile.h"

struct WZFileEntry{
	WZFileEntry() {}
	WZFileEntry(string name, int size, int checksum, int offset) : m_sName(name), m_iSize(size), m_iChecksum(checksum), m_iOffset(offset) {}
	~WZFileEntry() {}

	string			m_sName;
	int				m_iSize;
	int				m_iChecksum;
	int				m_iOffset;
	int				m_iRealOffset;
	
	IMGFile			m_IMGFile;
};
