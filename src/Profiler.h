#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <cassert>

#include <string>
using std::string;

#include "Log.h"

#define PROFILE(s) Profiler sample(s)

const int MAX_PROFILER_SAMPLES = 64;

struct ProfileSample{
	ProfileSample()
	{
		m_sName				= string(32, ' ');
		m_bIsValid			= false;
		m_iRunCount			= 0;
		m_iDataCount		= 0;
		m_fMinPercentage	= -1;
		m_fMaxPercentage	= -1;
		m_fAvgPercentage	= -1;
		m_fAvgTicks			= -1;
	}

	string			m_sName;		
	bool			m_bIsValid;		
	bool			m_bIsOpen;

	//per-run data
	LARGE_INTEGER	m_iStartTime;
	LARGE_INTEGER	m_iTotalTime;
	LARGE_INTEGER	m_iChildTime;
	int				m_iRunCount;
	int				m_iDepth;

	//long-term data 
	float			m_fMinPercentage;
	float			m_fMaxPercentage;
	float			m_fAvgPercentage;
	float			m_fAvgTicks;
	int				m_iDataCount;
};

class Profiler{
public:
	Profiler(string sSampleName);
	~Profiler();

	static void ResetSample(string sSampleName);
	static void ResetAllSamples();
	static void Update();
	static void Output();

protected:
	static ProfileSample	m_vSamples[MAX_PROFILER_SAMPLES];

	int						m_iSampleIndex;
	int						m_iParentIndex;

	static int				m_iLastOpenedSample;	//gives parent index
	static int				m_iNumOpenSamples;		//gives depth
	static LARGE_INTEGER	m_iRootBeginTime;
	static LARGE_INTEGER	m_iRootEndTime;
};
