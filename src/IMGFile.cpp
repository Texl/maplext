#include "IMGFile.h"

//interface
void IMGFile::Open(string name, FILE* fileIn, int offset, bool bResolveUOLs)
{
	//PROFILE("IMG: open");

	m_fileIn = fileIn;
	m_sName = name;
	m_entryRoot.m_sIdentifier = name;
	m_iStartOffset = offset;

	{
		//PROFILE("IMG: parse");
	ParseExtended(&m_entryRoot);
	}

	if(bResolveUOLs)
	{
		//PROFILE("IMG: resolve UOLs");
	//ResolveUOLs(&m_entryRoot);
	}
}

void IMGFile::ExtractImages(IMGEntry *pEntry, string path)
{
	//PROFILE("IMG: extract images");

	if(m_iMaxDataLength > 0  && m_vImages.size() > 0)
		m_pData = new unsigned char[m_iMaxDataLength];
	else
		return;

	CreateDirectory(m_sName.c_str(), NULL);
	SetCurrentDirectory(m_sName.c_str());

	for(unsigned int i = 0; i < m_vImages.size(); i++)
	{
		Image* pImage = &m_vImages[i];
		fseek(m_fileIn, pImage->m_iDataOffset, SEEK_SET);
		fread(m_pData, pImage->m_iDataLength, 1, m_fileIn);
		PNGWriter::Write(pImage->m_sName, pImage->m_iHeight, pImage->m_iWidth, pImage->m_iDataLength, m_pData, pImage->m_format);
	}

	SetCurrentDirectory("..");

	if(m_pData)
		delete[] m_pData;
}

void IMGFile::ExtractSounds(IMGEntry *pEntry, string path)
{
	//PROFILE("IMG: extract sounds");

	if(m_iMaxDataLength > 0 && m_vSound.size() > 0)
		m_pData = new unsigned char[m_iMaxDataLength];
	else
		return;

	CreateDirectory(m_sName.c_str(), NULL);
	SetCurrentDirectory(m_sName.c_str());

	for(unsigned int i = 0; i < m_vSound.size(); i++)
	{
		Sound* pSound = &m_vSound[i];
		fseek(m_fileIn, pSound->m_iDataOffset, SEEK_SET);
		fread(m_pData, pSound->m_iDataLength, 1, m_fileIn);
		FILE* outFile = fopen(pSound->m_sName.c_str(), "wb");
		fwrite(m_pData, pSound->m_iDataLength, 1, outFile);
		fclose(outFile);
	}

	SetCurrentDirectory("..");

	if(m_pData)
		delete[] m_pData;
}

void IMGFile::ExportXML(FILE* fileOut, int depth, IMGEntry *pEntry)
{
	if(pEntry == NULL)
	{
		for(unsigned int iEntry = 0; iEntry < m_entryRoot.m_vEntries.size(); iEntry++)
			ExportXML(fileOut, depth, &m_entryRoot.m_vEntries[iEntry]);
	}
	else
	{
		string thisIndent;
		for(int i = 0; i < depth; i++)
			thisIndent += "\t";
		string childIndent = thisIndent + "\t";

		ostringstream tagOpen;
		ostringstream tagClose;

		tagOpen << thisIndent;
		tagClose << thisIndent;

		//create open and close XML tags
		switch(pEntry->m_eType){
		//unknown purpose
		case IMG_0x00:
			tagOpen << "<e0x00 name=\"" << pEntry->m_sIdentifier << "\"/>";
			break;

		//short
		case IMG_0x02:
			tagOpen << "<short name=\"" << pEntry->m_sIdentifier <<  "\" value=\"" << ((IMGData_0x02*)pEntry->m_pIMGData)->m_short <<"\"/>";
			break;

		//integer
		case IMG_0x03:
			tagOpen << "<int name=\"" << pEntry->m_sIdentifier << "\" value=\"" << ((IMGData_0x03*)pEntry->m_pIMGData)->m_int << "\"/>";
			break;

		//float
		case IMG_0x04:
			tagOpen << "<float name=\"" << pEntry->m_sIdentifier << "\" value=\"" << ((IMGData_0x04*)pEntry->m_pIMGData)->m_float << "\"/>";
			break;

		//double
		case IMG_0x05:
			tagOpen << "<double name=\"" << pEntry->m_sIdentifier << "\" value=\"" << ((IMGData_0x05*)pEntry->m_pIMGData)->m_double <<"\"/>";
			break;

		//string
		case IMG_0x08:
			tagOpen << "<string name=\"" << pEntry->m_sIdentifier << "\" value=\"" << ((IMGData_0x08*)pEntry->m_pIMGData)->m_string << "\"/>";
			break;

		//extended class
		case IMG_0x09:
			Log::Write("shouldn't be here..\n");
			break;

		//directory
		case IMG_property:
			tagOpen << "<imgdir name=\"" << pEntry->m_sIdentifier << "\">";
			tagClose << "</imgdir>";
			break;

		//image
		case IMG_canvas:
			tagOpen << "<canvas name=\"" << pEntry->m_sIdentifier 
					<< "\" w=\"" << ((IMGData_canvas*)pEntry->m_pIMGData)->m_iWidth
					<< "\" h=\"" << ((IMGData_canvas*)pEntry->m_pIMGData)->m_iHeight
					<< "\" size=\"" << ((IMGData_canvas*)pEntry->m_pIMGData)->m_iDataLength
					<< "\" offset=\"" << ((IMGData_canvas*)pEntry->m_pIMGData)->m_iDataOffset << "\">";
			tagClose << "</canvas>";
			break;

		//2D vector
		case IMG_vector:
			tagOpen << "<vector name=\"" << pEntry->m_sIdentifier 
					<< "\" x=\"" << ((IMGData_vector*)pEntry->m_pIMGData)->m_x 
					<< "\" y=\"" << ((IMGData_vector*)pEntry->m_pIMGData)->m_y << "\"/>";
			break;

		//collection of 2D vectors
		case IMG_convex:
			tagOpen << "<convex name=\"" << pEntry->m_sIdentifier << "\"/>";
			break;

		//sound
		case IMG_sound:
			tagOpen << "<sound name=\"" << pEntry->m_sIdentifier 
					<< "\" size=\"" << ((IMGData_sound*)pEntry->m_pIMGData)->m_iDataLength 
					<< "\" offset=\"" << ((IMGData_sound*)pEntry->m_pIMGData)->m_iDataOffset << "\"/>";
			break;

		//uniform object locator
		case IMG_UOL:
			tagOpen << "<uol name=\"" << pEntry->m_sIdentifier << "\" value=\"" << ((IMGData_UOL*)pEntry->m_pIMGData)->m_sUOL << "\"/>";
			break;

		//definitely shouldn't happen
		default:
			break;
		}

		//output open tag
		if(tagOpen.str() != thisIndent)
			fprintf(fileOut, "%s\n", tagOpen.str().c_str());

		//process sub-entries if present
		switch(pEntry->m_eType){
		case IMG_property:
		case IMG_canvas:
		case IMG_convex:
			for(unsigned int iEntry = 0; iEntry < pEntry->m_vEntries.size(); iEntry++)
			{
				ExportXML(fileOut, depth + 1, &(pEntry->m_vEntries[iEntry]));
			}
			break;

		default:
			//do nothing
			break;
		}

		//output close tag
		if(tagClose.str() != thisIndent)
			fprintf(fileOut, "%s\n", tagClose.str().c_str());
	}
}

//private members
void IMGFile::Parse(IMGEntry *pEntry, string path)
{
	//get entry identifier
	unsigned char marker = GetByte();
	switch(marker){
		case 0x00:
			pEntry->m_sIdentifier = DecodeString();
			break;
		case 0x01:
			pEntry->m_sIdentifier = DecodeStringAtOffset(GetInt() + m_iStartOffset);
			break;
		default:
			Log::Write("%08x: unhandled IMG identifier marker: %02x %08x %08x\n", ftell(m_fileIn) - 1 - m_iStartOffset, marker, ftell(m_fileIn), m_iStartOffset);
	}

	marker = GetByte();
	if(marker == 0x03)
	{
		//integer
		pEntry->m_eType = IMG_0x03;
		pEntry->m_pIMGData = new IMGData_0x03;
		((IMGData_0x03*)pEntry->m_pIMGData)->m_int = GetValue();
	}
	else if(marker == 0x08)
	{
		//string
		pEntry->m_eType = IMG_0x08;
		pEntry->m_pIMGData = new IMGData_0x08;
		marker = GetByte();
		if(marker == 0x00)
		{
			//literal string
			((IMGData_0x08*)pEntry->m_pIMGData)->m_string = DecodeString();
		}
		else if(marker == 0x01)
		{
			//referenced string
			((IMGData_0x08*)pEntry->m_pIMGData)->m_string = DecodeStringAtOffset(GetInt() + m_iStartOffset);
		}
//		else if(marker == 0x04)
//		{
//			//unknown purpose
//			GetInt();
//			GetInt();
//		}
		else
		{
			Log::Write("unknown string type\n");
		}
	}
	else if(marker == 0x04)
	{
		//float
		pEntry->m_eType = IMG_0x04;
		pEntry->m_pIMGData = new IMGData_0x04;
		((IMGData_0x04*)pEntry->m_pIMGData)->m_float = GetFloat();
	}
	else if(marker == 0x02)
	{
		//short
		pEntry->m_eType = IMG_0x02;
		pEntry->m_pIMGData = new IMGData_0x02;
		((IMGData_0x02*)pEntry->m_pIMGData)->m_short = GetShort();
	}
	else if(marker == 0x00)
	{
		//unknown purpose?
		pEntry->m_eType = IMG_0x00;
	}
	else if(marker == 0x05)
	{
		//double
		pEntry->m_eType = IMG_0x05;
		pEntry->m_pIMGData = new IMGData_0x05;
		((IMGData_0x05*)pEntry->m_pIMGData)->m_double = GetDouble();
	}
	else if(marker == 0x09)
	{
		//extended type
		pEntry->m_eType = IMG_0x09;
		GetInt();
		ParseExtended(pEntry, path);
	}
	else
	{
		pEntry->m_eType = IMG_unknownType;
		Log::Write("%08x: unknown IMGEntry type: %02x\n", ftell(m_fileIn) - 1 - m_iStartOffset, marker);
	}
}

void IMGFile::ParseExtended(IMGEntry *pEntry, string path)
{
	string sType;
	string newPath = path + pEntry->m_sIdentifier.c_str() + "-";

	//get class identifier
	unsigned char marker = GetByte();
	switch(marker){
		case 0x73:
			sType = DecodeString();
			break;
		case 0x1B:
			sType = DecodeStringAtOffset(GetInt() + m_iStartOffset);
			break;
		default:
			Log::Write("%08x: unhandled IMG identifier marker: %02x %08x %08x\n", ftell(m_fileIn) - 1 - m_iStartOffset, marker, ftell(m_fileIn), m_iStartOffset);
	}

	//determine class type
	if(sType == "Property")
	{
		pEntry->m_eType = IMG_property;
		GetByte();
		GetByte();
		pEntry->m_vEntries.resize(GetValue());
		for(unsigned int entry = 0; entry < pEntry->m_vEntries.size(); entry++)
		{
			pEntry->m_vEntries[entry].m_pParent = pEntry;
			Parse(&(pEntry->m_vEntries[entry]), newPath);
		}
	}
	else if(sType == "Shape2D#Vector2D")
	{
		pEntry->m_eType	= IMG_vector;
		pEntry->m_pIMGData = new IMGData_vector;
		IMGData_vector* pData = (IMGData_vector*)pEntry->m_pIMGData;
		pData->m_x	= GetValue();
		pData->m_y	= GetValue();
	}
	else if(sType == "Canvas")
	{
		pEntry->m_eType = IMG_canvas;
		pEntry->m_pIMGData = new IMGData_canvas;
		IMGData_canvas* pData = (IMGData_canvas*)pEntry->m_pIMGData;
		GetByte();
		marker = GetByte();
		if(marker == 0x01)
		{
			GetByte();
			GetByte();
			pEntry->m_vEntries.resize(GetValue());
			for(unsigned 
				int entry = 0; entry < pEntry->m_vEntries.size(); entry++)
			{
				pEntry->m_vEntries[entry].m_pParent = pEntry;
				Parse(&(pEntry->m_vEntries[entry]), newPath);
			}
		}
		else
		{
			pEntry->m_vEntries.clear();
		}

		//get image data
		pData->m_iWidth		= GetValue();
		pData->m_iHeight	= GetValue();
		int format1			= GetValue();
		int format2			= (int)GetByte();
		pData->m_format		= (ImageFormat)(format1 + format2);
		int temp2 = GetInt();
		pData->m_iDataLength	= GetInt() - 1;
		if(pData->m_iDataLength > m_iMaxDataLength)
			m_iMaxDataLength = pData->m_iDataLength;
		GetByte();
		pData->m_iDataOffset = ftell(m_fileIn);
		fseek(m_fileIn, pData->m_iDataLength, SEEK_CUR);

		Image image;
		image.m_sName		= path + pEntry->m_sIdentifier + ".png";
		image.m_iWidth		= pData->m_iWidth;
		image.m_iHeight		= pData->m_iHeight;
		image.m_format		= pData->m_format;
		image.m_iDataLength	= pData->m_iDataLength;
		image.m_iDataOffset	= pData->m_iDataOffset;
		m_vImages.push_back(image);
	}
	else if(sType == "Shape2D#Convex2D")
	{
		pEntry->m_eType	= IMG_convex;
		pEntry->m_vEntries.resize(GetValue());
		for(unsigned int entry = 0; entry < pEntry->m_vEntries.size(); entry++)
		{
			pEntry->m_vEntries[entry].m_pParent = pEntry;
			ParseExtended(&(pEntry->m_vEntries[entry]));
		}
	}
	else if(sType == "Sound_DX8")
	{
		pEntry->m_eType = IMG_sound;
		pEntry->m_pIMGData = new IMGData_sound;
		IMGData_sound* pData = (IMGData_sound*)pEntry->m_pIMGData;

		GetByte();
		pData->m_iDataLength = GetValue();
		GetValue(); //no clue what this is

		//the file starts here, but we don't know the header size (to tack onto the data size.)
		pData->m_iDataOffset = ftell(m_fileIn);

		fseek(m_fileIn, 51, SEEK_CUR);
		pData->m_iDataLength += GetValue();
		pData->m_iDataLength += ftell(m_fileIn) - pData->m_iDataOffset;
		if(pData->m_iDataLength > m_iMaxDataLength)
			m_iMaxDataLength = pData->m_iDataLength;

		fseek(m_fileIn, pData->m_iDataOffset, SEEK_SET);
		fseek(m_fileIn, pData->m_iDataLength, SEEK_CUR);

		Sound sound;
		sound.m_sName		= path + pEntry->m_sIdentifier + ".mp3";
		sound.m_iDataLength	= pData->m_iDataLength;
		sound.m_iDataOffset	= pData->m_iDataOffset;
		m_vSound.push_back(sound);
	}
	else if(sType == "UOL")
	{
		pEntry->m_eType = IMG_UOL;
		pEntry->m_pIMGData = new IMGData_UOL;
		IMGData_UOL* pData = (IMGData_UOL*)pEntry->m_pIMGData;

		GetByte();
		marker = GetByte();
		switch(marker){
		case 0x00:
			pData->m_sUOL = DecodeString();
			break;
		case 0x01:
			pData->m_sUOL = DecodeStringAtOffset(GetInt() + m_iStartOffset);
			break;
		}
	}
	else
	{
		pEntry->m_eType = IMG_unknownExtendedType;
		Log::Write("%08x: unhandled IMG entry type: %02x %08x\n", ftell(m_fileIn) - 1 - m_iStartOffset, pEntry->m_eType, ftell(m_fileIn));
	}
}

void IMGFile::ResolveUOLs(IMGEntry *pEntry)
{
	if(pEntry->m_eType == IMG_UOL)
	{
		IMGEntry *pResolvedEntry = pEntry->GetNodeByUOL(((IMGData_UOL*)pEntry->m_pIMGData)->m_sUOL);
		if(pResolvedEntry)
		{
			//copy node, sans name
			string name = pEntry->m_sIdentifier;
            *pEntry = *pResolvedEntry;
			pEntry->m_sIdentifier = name;
		}
		return;
	}

	for(unsigned int iEntry = 0; iEntry < pEntry->m_vEntries.size(); iEntry++)
	{
		ResolveUOLs(&(pEntry->m_vEntries[iEntry]));
	}
}

bool IMGFile::CullTree(IMGEntry *pEntry, eIMGFileType eType)
{
	bool ret = false;

	if(pEntry->m_eType == eType)
		ret = true;

	vector<IMGEntry>::iterator iEntry = pEntry->m_vEntries.begin();
	while(iEntry != pEntry->m_vEntries.end())
	{
		eIMGFileType entryType = (*iEntry).m_eType;
		if(entryType == eType)
		{
			ret = true;
			iEntry++;
		}
		else if(entryType == IMG_property)
		{
			ret |= CullTree(&(*iEntry), eType);
			if(!ret)
				iEntry = pEntry->m_vEntries.erase(iEntry);
			else
				iEntry++;
		}
		else
			iEntry = pEntry->m_vEntries.erase(iEntry);
	}
	return ret;
}

string IMGFile::DecodeStringAtOffset(int offset)
{
	int oldOffset = ftell(m_fileIn);
	fseek(m_fileIn, offset, SEEK_SET);
	string ret = DecodeString();
	fseek(m_fileIn, oldOffset, SEEK_SET);
	return ret;
}

string IMGFile::GetString(int length)
{
	string ret;
	int limit = length;
	if(limit == 0)
		limit = INT_MAX;

	for(int i = 0; i < limit; i++)
	{
		unsigned char byte = GetByte();
		if(byte == 0x00)
			break;

		ret += (char)byte;
	}

	return ret;
}

string IMGFile::DecodeString()
{
	//PROFILE("IMG: decode string");
	int strLength;
	string retStr;
	unsigned char byte = GetByte();

	if(byte == 0x00)
	{
		return "";
	}

	if(byte <= 0x7F)
	{
		//do unicode

		unsigned short umask = 0xAAAA;

		if(byte == 0x7F)
			strLength = GetInt();
		else
			strLength = (int)byte;

		if(strLength < 0)
		{
			int temp = ftell(m_fileIn);
			return "";
		}

		for(int i = 0; i < strLength; i++)
		{
			unsigned short character = GetByte();
			character += GetByte() * 256;

			character ^= umask;
			umask++;

			char code[6];
			itoa(character, code, 10);

			retStr += "&#" + string(code) + ";";
		}

		return retStr;
	}
	else
	{
		//do non-unicode

		unsigned char mask = 0xAA;
	
		if(byte == 0x80)
			strLength = GetInt();
		else
			strLength = -(int)((char)byte);

		if(strLength < 0)
			return "bad string";

		for(int i = 0; i < strLength; i++)
		{
			unsigned char byte = GetByte();
			byte ^= mask;
			mask++;
			retStr += (char)byte;
		}

		return retStr;
	}
}

unsigned char IMGFile::GetByte()
{
	unsigned char ret;
	fread(&ret, 1, 1, m_fileIn);
	return ret;
}

int IMGFile::GetValue()
{
	unsigned char byte;
	long int ret;

	fread(&byte, 1, 1, m_fileIn);
	if(byte == 0x80)
	{
		fread(&ret, 4, 1, m_fileIn);
		return (int)ret;
	}
	else
		return (int)(char)byte;
}

short IMGFile::GetShort()
{
	short ret;
	fread(&ret, 2, 1, m_fileIn);
	return ret;
}

int IMGFile::GetInt()
{
	long int ret;
	fread(&ret, 4, 1, m_fileIn);
	return (int)ret;
}

float IMGFile::GetFloat()
{
	unsigned char byte;
	float ret;

	fread(&byte, 1, 1, m_fileIn);
	if(byte == 0x80)
	{
		fread(&ret, 4, 1, m_fileIn);
		return ret;
	}
	else
		return 0;
}

double IMGFile::GetDouble()
{
	double ret;
	fread(&ret, 8, 1, m_fileIn);
	return ret;
}
