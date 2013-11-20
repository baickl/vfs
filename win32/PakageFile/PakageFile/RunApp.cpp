// RunApp.cpp: implementation of the CRunApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "RunApp.h"
#include <string>


std::wstring ConvertAnsiToUnicode(std::string str)
{
	std::wstring result;

	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if( len < 0 )
		return result;

	wchar_t* buffer = new wchar_t[len + 1];
	if( buffer == NULL )
		return result;

	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);

	buffer[len] = '\0';
	result = buffer;
	delete[] buffer;

	return result;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRunApp::CRunApp()
{
	m_strOutput = L"";
}

CRunApp::~CRunApp()
{

}

//////////////////////////////////////////////////////////////////////
// CRunApp::SafeCloseHandle
// close a handle safely
// return nonzero if close successful
//////////////////////////////////////////////////////////////////////
int CRunApp::SafeCloseHandle(HANDLE& hHandle)
{
	int nRT = 0;
	if(hHandle)
	{
		nRT = ::CloseHandle(hHandle);
		hHandle = NULL;
	}
	return nRT;
}

//////////////////////////////////////////////////////////////////////
// CRunApp::GetProcessRunTime
// get process user mode time
// The return value is the time of milliseconds
//////////////////////////////////////////////////////////////////////
DWORD CRunApp::GetProcessRunTime(HANDLE hProcess)
{
	FILETIME ct, et, kt, ut;
	if (GetProcessTimes(hProcess, &ct, &et, &kt, &ut)==0)
	{
		return 0;
	}
	return (ut.dwLowDateTime + 9600)/10000;
}

//////////////////////////////////////////////////////////////////////
// CRunApp::Run
// run a process and capture the output
//////////////////////////////////////////////////////////////////////
DWORD CRunApp::Run(
				   LPCTSTR szAppName,
				   LPCTSTR szCmdLine,
				   LPCTSTR szRunDir,
				   WORD wShowWindow,
				   DWORD dwTimeLimit)
{
	WCHAR sCmdline[RUNAPP_DEFAULT_PACK_SIZE+1] = L"";
	SECURITY_ATTRIBUTES security_attrib = {0};
	STARTUPINFO stInfo  = {0};
	HANDLE hReadPipe    = NULL;
	HANDLE hWritePipe   = NULL;
	LPTSTR pStr         = NULL;
	BOOL bRet           = FALSE;
	int nOK             = 0;
	DWORD dwReturn      = RUNAPP_ERROR_SUCCESS;

	//
	// initialize
	//
	m_strOutput = L"";
	pStr = new WCHAR[wcslen(szCmdLine)+1];
	wcscpy(pStr, szCmdLine);

	security_attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attrib.bInheritHandle = TRUE;

	::ZeroMemory(&m_procInfo, sizeof(PROCESS_INFORMATION));

	//
	// CreatePipe
	//
	if (! ::CreatePipe(
		&hReadPipe,
		&hWritePipe,
		&security_attrib,
		RUNAPP_DEFAULT_PIPE_SIZE))
	{
		dwReturn = RUNAPP_ERROR_CREATE_PIPE;
		goto CLEAR_MEM;
	}

	//
	// Set start up info
	//
	stInfo.cb           = sizeof(STARTUPINFO);
	stInfo.dwFlags      = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	stInfo.hStdOutput   = hWritePipe;
	stInfo.hStdError    = hWritePipe;

	//
	// CreateProcess
	//
	if (!::CreateProcess(
		szAppName,
		pStr,
		&security_attrib,
		&security_attrib,
		TRUE,
		CREATE_NEW_CONSOLE|CREATE_SUSPENDED|NORMAL_PRIORITY_CLASS,
		NULL,
		szRunDir,
		&stInfo,
		&m_procInfo))
	{
		dwReturn = RUNAPP_ERROR_CREATE_PROCESS;
		goto CLEAR_MEM;
	}

	nOK = SafeCloseHandle(hWritePipe);
	ASSERT_TRUE(nOK);

	//
	// begin run thread
	//
	ResumeThread(m_procInfo.hThread);

	//
	// Wait for main thread
	//
	while (WaitForSingleObject(m_procInfo.hThread, 500)==WAIT_TIMEOUT)
	{
		char szTemp[RUNAPP_DEFAULT_PACK_SIZE+1] = {0};
		//
		// Get process output
		//
		{
			DWORD dwLen = GetFileSize(
				hReadPipe,
				NULL);
			while(dwLen>RUNAPP_DEFAULT_PACK_SIZE)
			{
				memset(szTemp, 0, sizeof(szTemp));
				DWORD dwRead;
				bRet = ::ReadFile(
					hReadPipe,
					&szTemp,
					RUNAPP_DEFAULT_PACK_SIZE,
					&dwRead,
					0);
				m_strOutput = m_strOutput + ConvertAnsiToUnicode(szTemp).c_str();
				dwLen -= RUNAPP_DEFAULT_PACK_SIZE;
			}
		}
		//
		// if wait timeout
		//
		if(GetProcessRunTime(m_procInfo.hProcess)>dwTimeLimit)
		{
			::TerminateProcess(m_procInfo.hProcess, 0xFFFFFFFF);
			dwReturn = RUNAPP_ERROR_RUN_TIMEOUT;
			goto CLEAR_MEM;
		}
	}

	//
	// Get process run time
	//
	m_dwRunTime = GetProcessRunTime(m_procInfo.hProcess);

	//
	// Get process output
	//
	do
	{
		char szTemp[RUNAPP_DEFAULT_PACK_SIZE+1] = {0};
		memset(szTemp, 0, sizeof(szTemp));
		DWORD dwRead;
		bRet = ::ReadFile(
			hReadPipe,
			&szTemp,
			RUNAPP_DEFAULT_PACK_SIZE,
			&dwRead,
			0);

		szTemp[dwRead] = 0;

		m_strOutput += ConvertAnsiToUnicode(szTemp).c_str();
	} while (bRet != 0 );

CLEAR_MEM:
	//
	// clear memory
	//
	if (pStr)
	{
		delete[] pStr;
		pStr = NULL;
	}

	nOK = SafeCloseHandle(hWritePipe);
	nOK = SafeCloseHandle(hReadPipe);
	nOK = SafeCloseHandle(m_procInfo.hThread);
	nOK = SafeCloseHandle(m_procInfo.hProcess);

	return dwReturn;
}
