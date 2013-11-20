#ifndef __MY_THREAD_H__
#define __MY_THREAD_H__

#include "FastDelegate.h"

#include <process.h>
#include <windows.h>


//使用名字空间
namespace mylib{

///////////////////////////////////////////////////////////////////////////////
//临界区的简易封装
///////////////////////////////////////////////////////////////////////////////
class CCriticalSection{ 
public: 
	CCriticalSection() { 
		::InitializeCriticalSection(&m_cs); 
	} 

	~CCriticalSection() { 
		::DeleteCriticalSection(&m_cs); 
	} 

	virtual void Lock() {
		::EnterCriticalSection(&m_cs); 
	} 

	virtual void Unlock() { 
		::LeaveCriticalSection(&m_cs);
	} 

private:
	CRITICAL_SECTION m_cs; 
}; 


///////////////////////////////////////////////////////////////////////////////
//线程类简易封装，不推荐继承本类
///////////////////////////////////////////////////////////////////////////////
class CAutoGuard{ 
public: 
	CAutoGuard(CCriticalSection* pObject) : m_pObject(pObject) {
		if( m_pObject )
			m_pObject->Lock(); 
	}

	~CAutoGuard() {
		if( m_pObject )
			m_pObject->Unlock();
	}
	
private:

	CAutoGuard(const CAutoGuard&);
	CAutoGuard& operator =(const CAutoGuard&);

private: 

	CCriticalSection* m_pObject; 
};


///////////////////////////////////////////////////////////////////////////////
//线程类简易封装，不推荐继承本类
///////////////////////////////////////////////////////////////////////////////
class CThread{
public:

	typedef fastdelegate::FastDelegate<void()> _ThreadBegin;
	typedef fastdelegate::FastDelegate<void()> _ThreadEnd;
	typedef fastdelegate::FastDelegate<bool()> _ThreadExecute;

public:

	//构造函数
	CThread(){
		m_hEventSuspend = CreateEvent(0,TRUE,FALSE,NULL);//创建事件句柄,且为非激发状态
		m_bExecute = true;

		//创建线程
		m_hThread = (HANDLE)_beginthreadex(NULL,0,_ThreadProc,this,0,&m_thrdaddr);
	}

	//析构函数
	~CThread(){
		if( m_hEventSuspend ){
			CloseHandle(m_hEventSuspend);
			m_hEventSuspend = NULL;
		}

		if( m_hThread ){
			TerminateThread(m_hThread,0);				//强制退出线程

			CloseHandle(m_hThread );
			m_hThread  = NULL;
		}
	}


public:

	//启动
	BOOL Start(){
		return SetEvent(m_hEventSuspend);
	}

	//挂起
	BOOL Suspend(){
		return ResetEvent(m_hEventSuspend);
	}

	//停止
	BOOL Stop(){
		m_bExecute = false;							//将线程状态置为退出
		Start();									//把线程唤醒,以免挂死在休眠处
		WaitForSingleObject(m_hThread,300);			//等待线程退出

		return TRUE;
	}

public:

	//线程回调函数相关

	_ThreadBegin	ThreadBegin;
	_ThreadEnd		ThreadEnd;

	_ThreadExecute	ThreadExecute;						//线程执行代码	

private:

	HANDLE			m_hThread;							//线程句柄
	HANDLE			m_hEventSuspend;					//用于标识线程是工作,还是挂起状态
	bool			m_bExecute;							//线程状态,FALSE表示线程退出状态,TRUE表示未提出状态
	unsigned		m_thrdaddr;

private:

	static unsigned __stdcall _ThreadProc( void * pObj){
		CThread* pthread = static_cast<CThread*>(pObj);

		//线程启动事件
		if( pthread && !pthread->ThreadBegin.empty())
			pthread->ThreadBegin();

		//以下线程循环
		while( pthread && pthread->m_bExecute ){
			WaitForSingleObject(pthread->m_hEventSuspend,INFINITE);//控制挂起，运行

			//如果自动退出
			if(pthread->ThreadExecute.empty())
				break;

			//执行结果
			if(!pthread->ThreadExecute())
				break;
		}

		//线程退出
		pthread->m_bExecute = false;

		//线程结束事件
		if( pthread && !pthread->ThreadEnd.empty())
			pthread->ThreadEnd();

		_endthreadex(0);
		return 0;
	}
};

}//namespace mylib

#endif//__MY_THREAD_H__