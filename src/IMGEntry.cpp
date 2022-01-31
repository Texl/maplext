#include "IMGEntry.h"

IMGEntry::IMGEntry()
{
	m_eType		= IMG_none;
	m_pParent	= NULL;
	m_pIMGData	= NULL;
}

IMGEntry::~IMGEntry()
{
	if(m_pIMGData)
//		m_pIMGData = NULL;
 		delete m_pIMGData;
}

IMGEntry* IMGEntry::GetNodeByName(string sName, bool bRecurse)
{
	IMGEntry* pRet = NULL;

	//look for the name, add any found to the list
	for(unsigned int iEntry = 0; iEntry < m_vEntries.size(); iEntry++)
	{
			if(stricmp(m_vEntries[iEntry].m_sIdentifier, sName))
			return &m_vEntries[iEntry];
	}

	if(bRecurse)
	{
		//process children, add results to the list
		for(unsigned int iEntry = 0; iEntry < m_vEntries.size(); iEntry++)
		{
			pRet = m_vEntries[iEntry].GetNodeByName(sName);
			if(pRet)
				return pRet;
		}
	}

	return NULL;
}

IMGEntry* IMGEntry::GetNodeByClass(eIMGFileType eType, bool bRecurse)
{
	IMGEntry* pRet = NULL;

	//look for the class, add any found to the list
	for(unsigned int iEntry = 0; iEntry < m_vEntries.size(); iEntry++)
	{
		if(m_vEntries[iEntry].m_eType == eType)
			return &m_vEntries[iEntry];
	}

	if(bRecurse)
	{
		//process children, add results to the list
		for(unsigned int iEntry = 0; iEntry < m_vEntries.size(); iEntry++)
		{
			pRet = m_vEntries[iEntry].GetNodeByClass(eType);
			if(pRet)
				return pRet;
		}
	}

	return NULL;
}

IMGEntry* IMGEntry::GetNodeByUOL(string sUOL)
{
	vector<string> UOLtokens;
	Tokenize(sUOL, UOLtokens, "/");

	IMGEntry *pRet = m_pParent;

	if(UOLtokens[0] != ".." && UOLtokens.size() != 1)
	{
		pRet = pRet->m_pParent;
	}

	for(unsigned int iToken = 0; iToken < UOLtokens.size(); iToken++)
	{	
		if(UOLtokens[iToken] == "..")
			pRet = pRet->m_pParent;
		else
			pRet = pRet->GetNodeByName(UOLtokens[iToken], false);

		if(pRet == NULL)
			return NULL;
	}

	if(pRet->m_eType == IMG_UOL)
	{
		pRet = pRet->GetNodeByUOL(((IMGData_UOL*)pRet->m_pIMGData)->m_sUOL);
	}

	return pRet;
}

