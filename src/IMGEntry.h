#pragma once

#include "Globals.h"
#include "PNGWriter.h"

enum eIMGFileType{
	IMG_none,
	IMG_0x00,
	IMG_0x02,
	IMG_0x03,	//integer
	IMG_0x04,	//float
	IMG_0x05,
	IMG_0x08,	//string
	IMG_0x09,	//extended type
	IMG_property,
	IMG_canvas,
	IMG_vector,
	IMG_convex,
	IMG_sound,
	IMG_UOL,
	IMG_unknownType,
	IMG_unknownExtendedType
};

struct IMGDataBase;

struct IMGEntry{
	IMGEntry();
	~IMGEntry();

	string				m_sIdentifier;
	eIMGFileType		m_eType;
	IMGDataBase*		m_pIMGData;
	IMGEntry*			m_pParent;
	vector<IMGEntry>	m_vEntries;

	IMGEntry*			GetNodeByName(string sName,  bool bRecurse = true);
	IMGEntry*			GetNodeByClass(eIMGFileType eType,  bool bRecurse = true);
	IMGEntry*			GetNodeByUOL(string sUOL);
};

struct IMGDataBase{
	IMGDataBase() {}
	virtual ~IMGDataBase() {}
};

//struct IMGData_0x00 : public IMGDataBase{
//};

struct IMGData_0x02 : public IMGDataBase{
	short m_short;
};

struct IMGData_0x03 : public IMGDataBase{
	int m_int;
};

struct IMGData_0x04 : public IMGDataBase{
	float m_float;
};

struct IMGData_0x05 : public IMGDataBase{
	double m_double;
};

struct IMGData_0x08 : public IMGDataBase{
	string m_string;
};

//struct IMGData_property : public IMGDataBase{
//};

struct IMGData_canvas : public IMGDataBase{
	int			m_iHeight;
	int			m_iWidth;
	int			m_iDataLength;
	int			m_iDataOffset;
	ImageFormat	m_format;
};

struct IMGData_vector : public IMGDataBase{
	int m_x;
	int m_y;
};

//struct IMGData_convex : public IMGDataBase{
//};

struct IMGData_sound : public IMGDataBase{
	int	m_iDataLength;
	int	m_iDataOffset;
};

struct IMGData_UOL : public IMGDataBase{
	string m_sUOL;
};
