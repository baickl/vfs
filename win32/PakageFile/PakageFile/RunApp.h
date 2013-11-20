#ifndef JUMPING_RUNAPP_H
#define JUMPING_RUNAPP_H

#pragma once

#include <afxwin.h>

#define RUNAPP_DEFAULT_PIPE_SIZE        0x0
#define RUNAPP_DEFAULT_PACK_SIZE        0x100
#define RUNAPP_ERROR_CREATE_PIPE        0xFFFFFFFF
#define RUNAPP_ERROR_CREATE_PROCESS     0xFFFFFFFE
#define RUNAPP_ERROR_RUN_TIMEOUT        0x1
#define RUNAPP_ERROR_SUCCESS            0x0

#define RAISE_EXCEPTION         __asm{int 13}

#ifdef _DEBUG
#define ASSERT_TRUE(exp) if(!exp)RAISE_EXCEPTION
#else
#define ASSERT_TRUE(exp) ((void)0)
#endif

class CRunApp
{
private:
	CString                 m_strOutput;
	PROCESS_INFORMATION     m_procInfo;
	DWORD                   m_dwRunTime;
public:
	CRunApp();
	virtual ~CRunApp();
public:
	DWORD GetProcessRunTime(HANDLE hProcess);
	DWORD Run(
		LPCTSTR szAppName,
		LPCTSTR szCmdLine,
		LPCTSTR szRunDir  = NULL,
		WORD wShowWindow  = SW_SHOW,
		DWORD dwTimeLimit = INFINITE);
	int SafeCloseHandle(HANDLE& hHandle);
	operator CString()const{return m_strOutput;}
};

#endif