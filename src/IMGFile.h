#pragma once

#include "Globals.h"
#include "PNGWriter.h"

#include "IMGEntry.h"

struct Image{
	string		m_sName;
	int			m_iHeight;
	int			m_iWidth;
	int			m_iDataLength;
	int			m_iDataOffset;
	ImageFormat	m_format;
};

struct Sound{
	string		m_sName;
	int			m_iDataLength;
	int			m_iDataOffset;
};

class IMGFile{
public:
	IMGFile() : m_pData(NULL), m_iMaxDataLength(0) {}
	~IMGFile() {}

	void Open(string name, FILE* fileIn, int offset = 0, bool bResolveUOLs = false);
	void ExportXML(FILE* fileOut, int depth = 0, IMGEntry *pEntry = NULL);
	void ExtractImages(IMGEntry *pEntry = NULL, string path = "");
	void ExtractSounds(IMGEntry *pEntry = NULL, string path = "");

private:
	void Parse(IMGEntry *pEntry, string path = "");
	void ParseExtended(IMGEntry *pEntry, string path = "");

	void ResolveUOLs(IMGEntry *pEntry);
	bool CullTree(IMGEntry *pEntry, eIMGFileType eType);

	string DecodeStringAtOffset(int offset);
	string GetString(int length = 0);
	string DecodeString();
	unsigned char GetByte();
	int GetValue();
	short GetShort();
	int GetInt();
	float GetFloat();
	double GetDouble();

	string			m_sName;
	FILE*			m_fileIn;
	int				m_iStartOffset;
	IMGEntry		m_entryRoot;

	int				m_iMaxDataLength;
	unsigned char*	m_pData;

	vector<Image>	m_vImages;
	vector<Sound>	m_vSound;

//	friend class WZFile;
};
