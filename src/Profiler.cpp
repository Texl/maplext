#include "Profiler.h"

Profiler::Profiler(string sSampleName)
{
	//creates/resumes a profile sample record

	int iStoreIndex = -1;

	for(int i = 0; i < MAX_PROFILER_SAMPLES; ++i)
	{
		if(!m_vSamples[i].m_bIsValid && iStoreIndex < 0)
		{
			//get the index of the first unused sample slot in case this is a new sample
			iStoreIndex = i;
		}
		else if(m_vSamples[i].m_sName == sSampleName)
		{
			//sample name was found; make sure it isn't already open
			assert(!m_vSamples[i].m_bIsOpen);
			iStoreIndex = i;
			break;
		}
	}

	//make sure we're not out of sample slots
	assert(iStoreIndex >= 0);

	ProfileSample* pSample = &m_vSamples[iStoreIndex];

	if(!pSample->m_bIsValid)
	{
		m_vSamples[iStoreIndex].m_bIsValid = true;
		m_vSamples[iStoreIndex].m_sName = sSampleName;
		pSample->m_iTotalTime.QuadPart = 0;
		pSample->m_iChildTime.QuadPart = 0;
	}

	m_iSampleIndex = iStoreIndex;
	m_iParentIndex = m_iLastOpenedSample;
	m_iLastOpenedSample = iStoreIndex;

	pSample->m_iDepth = m_iNumOpenSamples;
	++m_iNumOpenSamples;
	pSample->m_bIsOpen = true;
	++pSample->m_iRunCount;

	QueryPerformanceCounter(&pSample->m_iStartTime);
	if(m_iParentIndex < 0)
		m_iRootBeginTime = pSample->m_iStartTime;

//	Log::Write("constructor for %s; stored index is %d, run %d times\n", pSample->m_sName.c_str(), m_iSampleIndex, pSample->m_iRunCount);
}

Profiler::~Profiler()
{
	LARGE_INTEGER iEndTime;
	QueryPerformanceCounter(&iEndTime);

	ProfileSample* pSample = &m_vSamples[m_iSampleIndex];
	pSample->m_bIsOpen = false;

//	Log::Write("destructor for %s; stored index is %d, run %d times\n", pSample->m_sName.c_str(), m_iSampleIndex, pSample->m_iRunCount);

	LARGE_INTEGER iElapsedTime;
	iElapsedTime.QuadPart = iEndTime.QuadPart - pSample->m_iStartTime.QuadPart;

	if(m_iParentIndex < 0)
	{
		m_iRootEndTime = iEndTime;
	}
	else
	{
		m_vSamples[m_iParentIndex].m_iChildTime.QuadPart += iElapsedTime.QuadPart;
	}

	pSample->m_iTotalTime.QuadPart += iElapsedTime.QuadPart;
	m_iLastOpenedSample = m_iParentIndex;
	--m_iNumOpenSamples;
}

void Profiler::ResetSample(string sSampleName)
{
	for(int i = 0; i < MAX_PROFILER_SAMPLES; ++i)
	{
		if((m_vSamples[i].m_bIsValid) && (m_vSamples[i].m_sName == sSampleName))
		{
			//sample name was found
			m_vSamples[i].m_fMinPercentage = -1;
			m_vSamples[i].m_fMaxPercentage = -1;
			m_vSamples[i].m_fAvgPercentage = -1;
			m_vSamples[i].m_iDataCount = 0;
			return;
		}
	}
}

void Profiler::ResetAllSamples()
{
	for(int i = 0; i < MAX_PROFILER_SAMPLES; ++i)
	{
		if(m_vSamples[i].m_bIsValid)
		{
			m_vSamples[i].m_fMinPercentage = -1;
			m_vSamples[i].m_fMaxPercentage = -1;
			m_vSamples[i].m_fAvgPercentage = -1;
			m_vSamples[i].m_iDataCount = 0;
		}
	}
}

void Profiler::Update()
{
	for(int i = 0; i < MAX_PROFILER_SAMPLES; ++i)
	{
		if(m_vSamples[i].m_bIsValid)
		{
			//find the sample's run time
			LARGE_INTEGER iSampleTime;
			iSampleTime.QuadPart = m_vSamples[i].m_iTotalTime.QuadPart - m_vSamples[i].m_iChildTime.QuadPart;

			//find the percentage of the root frame
			float fPercentage = (float)(iSampleTime.QuadPart) / (float)(m_iRootEndTime.QuadPart - m_iRootBeginTime.QuadPart) * 100.0f;

			//update trackers
			float fTotal = m_vSamples[i].m_fAvgPercentage * m_vSamples[i].m_iDataCount;
			fTotal += fPercentage;

			float fTicks = m_vSamples[i].m_fAvgTicks * (float)m_vSamples[i].m_iDataCount;
			fTicks += (float)iSampleTime.QuadPart / (float)m_vSamples[i].m_iRunCount;

			++m_vSamples[i].m_iDataCount;
			m_vSamples[i].m_fAvgTicks = fTicks / (float)m_vSamples[i].m_iDataCount;
			m_vSamples[i].m_fAvgPercentage = fTotal / (float)m_vSamples[i].m_iDataCount;

			if((m_vSamples[i].m_fMinPercentage == -1) || (fPercentage < m_vSamples[i].m_fMinPercentage))
				m_vSamples[i].m_fMinPercentage = fPercentage;

			if((m_vSamples[i].m_fMaxPercentage == -1) || (fPercentage > m_vSamples[i].m_fMaxPercentage))
				m_vSamples[i].m_fMaxPercentage = fPercentage;


			//reset sample for next loop
//			m_vSamples[i].m_iRunCount = 0;
			m_vSamples[i].m_iTotalTime.QuadPart = 0;
			m_vSamples[i].m_iChildTime.QuadPart = 0;
		}
	}
}

void Profiler::Output()
{
	LARGE_INTEGER iFrequency;
	QueryPerformanceFrequency(&iFrequency);
	LONGLONG lTicksPerSecond = iFrequency.QuadPart;

	Log::Write("  Min :   Avg :   Max :    Avg Ticks :  #Run : Name\n");
	Log::Write("---------------------------------------------------\n");

	char name[256], indentedName[256];
	char avg[16], min[16], max[16], num[16], count[16];

	for(int i = 0; i < MAX_PROFILER_SAMPLES; ++i)
	{
		if(m_vSamples[i].m_bIsValid)
		{
			sprintf(avg, "%3.1f", m_vSamples[i].m_fAvgPercentage);
			sprintf(min, "%3.1f", m_vSamples[i].m_fMinPercentage);
			sprintf(max, "%3.1f", m_vSamples[i].m_fMaxPercentage);
			sprintf(num, "%10.1f", m_vSamples[i].m_fAvgTicks);
//			sprintf(num, "%3.1f", m_vSamples[i].m_fAvgTicks / (float)lTicksPerSecond * 1000.0f);
			sprintf(count, "%3d", m_vSamples[i].m_iRunCount);

			strcpy(indentedName, m_vSamples[i].m_sName.c_str());
			for(int indent = 0; indent < m_vSamples[i].m_iDepth; ++indent)
			{
				sprintf(name, "  %s", indentedName);
				strcpy(indentedName, name);
			}
			Log::Write("%5s : %5s : %5s : %12s : %5s : %5s\n", min, avg, max, num, count, indentedName);
		}
	}

	Log::Write("Total runtime: %10.1fs\n", (float)(m_iRootEndTime.QuadPart - m_iRootBeginTime.QuadPart) / (float)lTicksPerSecond);
	Log::Write("\n");
}

ProfileSample			Profiler::m_vSamples[MAX_PROFILER_SAMPLES];
int						Profiler::m_iLastOpenedSample	= -1;
int						Profiler::m_iNumOpenSamples		= 0;
LARGE_INTEGER			Profiler::m_iRootBeginTime		= LARGE_INTEGER();
LARGE_INTEGER			Profiler::m_iRootEndTime		= LARGE_INTEGER();
