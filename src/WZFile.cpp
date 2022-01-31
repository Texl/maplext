#include "WZFile.h"

WZFile::WZFile()
{
	m_fileIn = NULL;
	m_dirRoot.m_sName = "root";
}

WZFile::WZFile(string filename)
{
	*this = WZFile();
	m_sFilename = filename;
	Open(filename);
}

WZFile::~WZFile() {}

//interface

bool WZFile::Open(string filename, string filter)
{
	//PROFILE("WZ: open");

	m_sFilter = filter;
	m_sFilename = filename;
	m_dirRoot.m_sName = filename;

	Close();

	m_fileIn = fopen(m_sFilename.c_str(), "rb");

	//parse file; create file structure
	m_sPKG = GetString(4);
	m_iFileSize = GetInt();
	m_iFileSize += GetInt() * INT_MAX;
	m_iHeaderSize = GetInt();
	m_sCopyright = GetString();
	m_iVersion = FindVersion();
	GetByte();
	ParseDirectory(&m_dirRoot);

	//find file offsets; match up with directory structure
	int offset = ftell(m_fileIn);
	FindOffsets(&m_dirRoot, offset);

	WZDirectory* pDir = NULL;
	if(m_sFilter != "")
		pDir = m_dirRoot.GetDirectoryByName(m_sFilter);
	if(pDir)
		ReadFiles(pDir);
	else
		ReadFiles(&m_dirRoot);

	return true;
}

void WZFile::Close()
{
	if(m_fileIn)
		fclose(m_fileIn);
}

void WZFile::ListDirectory(WZDirectory *pDirectory, string path)
{
	if(pDirectory == NULL)
	{
		//PROFILE("WZ: list directory");
		ListDirectory(&m_dirRoot);
	}
	else
	{
		string newPath = path + pDirectory->m_sName + "\\";
		Log::Write("%s\n", newPath.c_str());

		for(unsigned int iFile = 0; iFile < pDirectory->m_vFiles.size(); iFile++)
		{
			WZFileEntry* pFile = &(pDirectory->m_vFiles[iFile]);
			//char soffset[33];
			//itoa(file.m_iRealOffset, soffset, 2);
			//Log::Write("\t%-27s  %08x B  %08x c  %08x - %032s\n", file.m_sName.c_str(), file.m_iSize, file.m_iChecksum, file.m_iRealOffset, soffset); 

			char soffset[33];
			char sroffset[33];
			char sxoroffset[33];
			itoa(pFile->m_iOffset, soffset, 2);
			itoa(pFile->m_iRealOffset, sroffset, 2);
			itoa(pFile->m_iOffset ^ pFile->m_iRealOffset, sxoroffset, 2);

			Log::Write("\t%-27s  %08x bytes : %08x - %032s | %08x - %032s | %08x - %032s\n", 
				pFile->m_sName.c_str(), pFile->m_iSize, pFile->m_iOffset, soffset, pFile->m_iRealOffset, sroffset, pFile->m_iOffset ^ pFile->m_iRealOffset, sxoroffset); 
		}

		for(unsigned int iDir = 0; iDir < pDirectory->m_vSubdirectories.size(); iDir++)
		{
			ListDirectory(&(pDirectory->m_vSubdirectories[iDir]), newPath);
		}
	}
}

void WZFile::ExportXML(FILE* fileOut, WZDirectory *pDirectory, string path, int depth)
{
	if(pDirectory == NULL)
	{
		//PROFILE("WZ: export XML");

		Log::Write("Exporting XML...\n");

		fprintf(fileOut, "<WZarchive name=\"%s\" version=\"%d\">", m_sFilename.c_str(), m_iVersion);

		WZDirectory* pDir = NULL;
		if(m_sFilter != "")
			pDir = m_dirRoot.GetDirectoryByName(m_sFilter);
		if(pDir)
			ExportXML(fileOut, pDir, pDir->m_sName + "\\", 1);
		else
			ExportXML(fileOut, &m_dirRoot);

		fprintf(fileOut, "</WZarchive>");

		Log::Write("done!\n");
	}
	else
	{
		string newPath = path + pDirectory->m_sName + "\\";
		Log::Write("exporting: %s\n", newPath.c_str());

		string thisIndent;
		for(int i = 0; i < depth; i++)
			thisIndent += "\t";
		string childIndent = thisIndent + "\t";

		string elementTagOpen = thisIndent + "<directory name=\"" + pDirectory->m_sName + "\">\n";
		fprintf(fileOut, elementTagOpen.c_str());

		//process files in directory
		for(unsigned int iFile = 0; iFile < pDirectory->m_vFiles.size(); iFile++)
		{
			if(iFile % 10 == 0)
			{
				int progress = (iFile * 25) / (int)pDirectory->m_vFiles.size();
				printf("\r");
				for(int i = 0; i < progress; i++)
					printf("|");
			}

			WZFileEntry* pFile = &(pDirectory->m_vFiles[iFile]);
			string elementTagOpen = childIndent + "<file name=\"" + pFile->m_sName + "\">\n";
			fprintf(fileOut, elementTagOpen.c_str());

			//process file
			pFile->m_IMGFile.ExportXML(fileOut, depth + 2);

			string elementTagClose = childIndent + "</file>\n";
			fprintf(fileOut, elementTagClose.c_str());
		}
		printf("\r");
		Log::Write("|||||||||||||||||||||||||\n");

		//process child directories
		for(unsigned int iDir = 0; iDir < pDirectory->m_vSubdirectories.size(); iDir++)
		{
			ExportXML(fileOut, &(pDirectory->m_vSubdirectories[iDir]), newPath, depth + 1);
		}

		string elementTagClose = thisIndent + "</directory>\n";
		fprintf(fileOut, elementTagClose.c_str());
	}
}

void WZFile::ExtractImages(WZDirectory *pDirectory, string path)
{
	if(pDirectory == NULL)
	{
		//PROFILE("WZ: extract images");

		PNGWriter::Initialize();

		Log::Write("Extracting images...\n");

		WZDirectory* pDir = NULL;
		if(m_sFilter != "")
			pDir = m_dirRoot.GetDirectoryByName(m_sFilter);
		if(pDir)
			ExtractImages(pDir);
		else
			ExtractImages(&m_dirRoot);

		PNGWriter::Deinitialize();

		Log::Write("done!\n");
	}
	else
	{
		{
		//PROFILE("WZ extract images internal");

		string newPath = path + pDirectory->m_sName + "\\";
		Log::Write("dir create: %s\n", newPath.c_str());

		CreateDirectory(pDirectory->m_sName.c_str(), NULL);
		SetCurrentDirectory(pDirectory->m_sName.c_str());

		for(unsigned int iFile = 0; iFile < pDirectory->m_vFiles.size(); iFile++)
		{
			if(iFile % 10 == 0)
			{
				int progress = (iFile * 25) / (int)pDirectory->m_vFiles.size();
				printf("\r");
				for(int i = 0; i < progress; i++)
					printf("|");
			}

			WZFileEntry* pFile = &(pDirectory->m_vFiles[iFile]);
			pFile->m_IMGFile.ExtractImages();
		}
		printf("\r");
		Log::Write("|||||||||||||||||||||||||\n");

		}

		for(unsigned int iDir = 0; iDir < pDirectory->m_vSubdirectories.size(); iDir++)
		{
			ExtractImages(&(pDirectory->m_vSubdirectories[iDir]), path);
		}

		SetCurrentDirectory("..");
		RemoveDirectory(pDirectory->m_sName.c_str());
	}
}

void WZFile::ExtractSounds(WZDirectory *pDirectory, string path)
{
	if(pDirectory == NULL)
	{
		Log::Write("Extracting sounds...\n");

		WZDirectory* pDir = NULL;
		if(m_sFilter != "")
			pDir = m_dirRoot.GetDirectoryByName(m_sFilter);
		if(pDir)
			ExtractSounds(pDir);
		else
			ExtractSounds(&m_dirRoot);

		Log::Write("done!\n");
	}
	else
	{
		string newPath = path + pDirectory->m_sName + "\\";
		Log::Write("dir create: %s\n", newPath.c_str());

		CreateDirectory(pDirectory->m_sName.c_str(), NULL);
		SetCurrentDirectory(pDirectory->m_sName.c_str());

		for(unsigned int iFile = 0; iFile < pDirectory->m_vFiles.size(); iFile++)
		{
			if(iFile % 10 == 0)
			{
				int progress = (iFile * 25) / (int)pDirectory->m_vFiles.size();
				printf("\r");
				for(int i = 0; i < progress; i++)
					printf("|");
			}

			WZFileEntry* pFile = &(pDirectory->m_vFiles[iFile]);
			pFile->m_IMGFile.ExtractSounds();
		}
		printf("\r");
		Log::Write("|||||||||||||||||||||||||\n");
		
		for(unsigned int iDir = 0; iDir < pDirectory->m_vSubdirectories.size(); iDir++)
		{
			ExtractSounds(&(pDirectory->m_vSubdirectories[iDir]), newPath);
		}

		SetCurrentDirectory("..");
		RemoveDirectory(pDirectory->m_sName.c_str());
	}
}

void WZFile::ExtractFiles(WZDirectory *pDirectory, string path)
{
	if(pDirectory == NULL)
	{
		//PROFILE("WZ: extract files");
		Log::Write("Extracting files...\n");
		ExtractFiles(&m_dirRoot);
		Log::Write("done!\n");
	}
	else
	{
		string newPath = path + pDirectory->m_sName + "\\";
		Log::Write("dir create: %s\n", newPath.c_str());

		CreateDirectory(pDirectory->m_sName.c_str(), NULL);
		SetCurrentDirectory(pDirectory->m_sName.c_str());

		for(unsigned int iFile = 0; iFile < pDirectory->m_vFiles.size(); iFile++)
		{
			if(iFile % 10 == 0)
			{
				int progress = (iFile * 25) / (int)pDirectory->m_vFiles.size();
				printf("\r");
				for(int i = 0; i < progress; i++)
					printf("|");
			}

			WZFileEntry* pFile = &(pDirectory->m_vFiles[iFile]);

			unsigned char* pData = new unsigned char[pFile->m_iSize];
			fseek(m_fileIn, pFile->m_iRealOffset, SEEK_SET);
			fread(pData, pFile->m_iSize, 1, m_fileIn);

			FILE* outFile = fopen(pFile->m_sName.c_str(), "wb");
			fwrite(pData, pFile->m_iSize, 1, outFile);
			fclose(outFile);
			delete[] pData;

			//Log::Write("file write: %s\n", (newPath + fileEntry.m_sName).c_str()); 
		}
		printf("\r");
		Log::Write("|||||||||||||||||||||||||\n");

		for(unsigned int iDir = 0; iDir < pDirectory->m_vSubdirectories.size(); iDir++)
		{
			ExtractFiles(&(pDirectory->m_vSubdirectories[iDir]), newPath);
		}

		SetCurrentDirectory("..");
	}
}

string WZFile::DecodeStringAtOffset(int offset)
{
	int oldOffset = ftell(m_fileIn);
	fseek(m_fileIn, offset, SEEK_SET);
	string ret = DecodeString();
	fseek(m_fileIn, oldOffset, SEEK_SET);
	return ret;
}

//private members

int WZFile::FindVersion()
{
	unsigned char versionEncoded = GetByte();

	int sum;
	char versionStr[11];
	unsigned char a, b, c, d, e;
	for(unsigned int i = 0; i < 1000; i++) //would have made it INT_MAX, but I don't want a bottleneck
	{
		sum = 0;
		itoa(i, versionStr, 10);
        for(unsigned int j = 0; versionStr[j] != '\0'; j++)
		{
			sum <<= 5;
			sum += (unsigned char)versionStr[j] + 1;
		}
		a = (sum >> 24) & 0xFF;
		b = (sum >> 16) & 0xFF;
		c = (sum >> 8) & 0xFF;
		d = sum & 0xFF;
		e = 0xFF ^ a ^ b ^ c ^ d;

		if(versionEncoded == e)
			return i;
	}
	return 0;
}

void WZFile::ParseDirectory(WZDirectory *pDirectory)
{
	{{
	//PROFILE("WZ: parse directory");

	pDirectory->m_iRealOffset = ftell(m_fileIn);
	int entries = GetValue();

	for(int iEntry = 0; iEntry < entries; iEntry++)
	{
		unsigned char marker = GetByte();

		string name;
		int size, checksum, offset;

		switch(marker){
		case 0x02:
			name = DecodeStringAtOffset(GetInt() + m_iHeaderSize + 1);
			size = GetValue();
			checksum = GetValue();
			offset =  GetInt();
			pDirectory->m_vFiles.push_back(WZFileEntry(name, size, checksum, offset));
			break;

		case 0x03:
			name = DecodeString();
			size = GetValue();
			checksum = GetValue();
			offset =  GetInt();
			pDirectory->m_vSubdirectories.push_back(WZDirectory(name, size, checksum, offset));
			break;

		case 0x04:
			name = DecodeString();
			size = GetValue();
			checksum = GetValue();
			offset =  GetInt();
			pDirectory->m_vFiles.push_back(WZFileEntry(name, size, checksum, offset));
			break;
		}
	}

	}}

	for(int iDirectory = 0; iDirectory < (int)pDirectory->m_vSubdirectories.size(); iDirectory++)
	{
		ParseDirectory(&(pDirectory->m_vSubdirectories[iDirectory]));
	}
}

void WZFile::FindOffsets(WZDirectory *pDirectory, int &iStartOffset)
{
	{{
	//PROFILE("WZ: find offsets");

	for(unsigned int iFile = 0; iFile < pDirectory->m_vFiles.size(); iFile++)
	{
        (pDirectory->m_vFiles)[iFile].m_iRealOffset = iStartOffset;
		iStartOffset += (pDirectory->m_vFiles)[iFile].m_iSize;
	}

	}}

	for(unsigned int iDir = 0; iDir < pDirectory->m_vSubdirectories.size(); iDir++)
	{
		FindOffsets(&(pDirectory->m_vSubdirectories[iDir]), iStartOffset);
	}
}

void WZFile::ReadFiles(WZDirectory *pDirectory, string path)
{
	string newPath = path + pDirectory->m_sName + "\\";

	{{
	//PROFILE("WZ: read files");

	Log::Write("Reading %s:\n", newPath.c_str());

	for(unsigned int iFile = 0; iFile < pDirectory->m_vFiles.size(); iFile++)
	{
		if(iFile % 10 == 0)
		{
			int progress = (iFile * 25) / (int)pDirectory->m_vFiles.size();
			printf("\r");
			for(int i = 0; i < progress; i++)
				printf("|");
		}

		WZFileEntry *pFile = &(pDirectory->m_vFiles[iFile]);

		fseek(m_fileIn, pFile->m_iRealOffset, SEEK_SET);
		string sName = pFile->m_sName.substr(0, pFile->m_sName.size() - 4);
		pFile->m_IMGFile.Open(sName, m_fileIn, ftell(m_fileIn));
	}
	printf("\r");
	Log::Write("|||||||||||||||||||||||||\n");

	}}

	for(unsigned int iDir = 0; iDir < pDirectory->m_vSubdirectories.size(); iDir++)
	{
		ReadFiles(&(pDirectory->m_vSubdirectories[iDir]), newPath);
	}
}

string WZFile::GetString(int length)
{
	if(!m_fileIn)
		return "";

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

string WZFile::DecodeString()
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

unsigned char WZFile::GetByte()
{
	if(!m_fileIn)
		return 0;

	unsigned char ret;
	fread(&ret, 1, 1, m_fileIn);
	return ret;
}

int WZFile::GetInt()
{
	if(!m_fileIn)
		return 0;

	long int ret;
	fread(&ret, 4, 1, m_fileIn);
	return (int)ret;
}

int WZFile::GetValue()
{
	if(!m_fileIn)
		return 0;

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
