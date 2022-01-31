#include "WZDirectory.h"

WZDirectory* WZDirectory::GetDirectoryByName(string sName)
{
	//look for the name, if found then return it
	for(unsigned int iDir = 0; iDir < m_vSubdirectories.size(); iDir++)
	{
		if(m_vSubdirectories[iDir].m_sName == sName)
			return &m_vSubdirectories[iDir];
	}

	//process children
	for(unsigned int iDir = 0; iDir < m_vSubdirectories.size(); iDir++)
	{
		WZDirectory* pDir = m_vSubdirectories[iDir].GetDirectoryByName(sName);
		if(pDir)
			return pDir;
	}

	//if it's not found by now then this node doesn't contain the search name
	return NULL;
}

WZFileEntry* WZDirectory::GetFileByName(string sName)
{
	//look for the name, if found then return it
	for(unsigned int iFile = 0; iFile < m_vFiles.size(); iFile++)
	{
		if(m_vFiles[iFile].m_sName == sName)
			return &m_vFiles[iFile];
	}

	//process children
	for(unsigned int iDir = 0; iDir < m_vSubdirectories.size(); iDir++)
	{
		WZFileEntry* pFile = m_vSubdirectories[iDir].GetFileByName(sName);
		if(pFile)
			return pFile;
	}

	//if it's not found by now then this node doesn't contain the search name
	return NULL;
}
