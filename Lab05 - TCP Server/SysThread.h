#pragma once
#include <windows.h>

class SysThread
{
public:
	SysThread(void);
	virtual ~SysThread();
	virtual bool start(void);
	virtual bool stop(unsigned int timeout = 0);
	inline volatile bool& isRunning(void)
	{
		return m_bRunning;
	}
	inline volatile bool& isExited(void)
	{
		return m_bExited;
	}

	inline volatile bool& isConnected(void)
	{
		return m_connected;
	}

	inline volatile void setToDisconnect(void)
	{
		m_connected = false;
	}
protected:
	virtual void run(void); //Ezt a metodust a származtatott
	//osztályban felül kell írni.Ide kell beírni az utasítás szekvenciát
	//	amit a szálunk végre kell hajtson

private:
	friend DWORD WINAPI runStub(LPVOID mthread);


public:
	static const unsigned int INFINIT_WAIT;
private:
	volatile bool m_bRunning;
	volatile bool m_bExited;
	volatile bool m_connected;
	HANDLE m_thread;
};


