#pragma once
#include "../Network/GNetworkGlobal.h"

/**
*	@brief		공유자원에 multi-read, single write를 위한 SRW Lock의 Wrapper 클래스 입니다.
*	@todo		일단 SRWLock의 성능이 만족스러우므로 직접 구현은 나~~~~~중에 하는걸로..
*/
class GRWLock
{
public:
	GRWLock();

	virtual ~GRWLock();

	/**
	*	@brief	 이 스레드에서 WriteLock을 시작합니다.
	*/
	inline void BeginWriteLock();

	/**
	*	@brief	이 스레드의 WriteLock을 중지합니다.
	*/
	inline void EndWriteLock();

	/**
	*	@brief	 이 스레드에서 ReadLock을 시작합니다.
	*/
	inline void BeginReadLock();

	/**
	*	@brief	 이 스레드에서 ReadLock을 중지합니다.
	*/
	inline void EndReadLock();

private:
	bool checkDeadlock();
	SRWLOCK SRWLockHandle;
	HANDLE LastThread;
};



inline void GRWLock::BeginWriteLock()
{
	if (checkDeadlock()) return;
	AcquireSRWLockExclusive(&SRWLockHandle);
}


inline void GRWLock::EndWriteLock()
{
	ReleaseSRWLockExclusive(&SRWLockHandle);
	LastThread = NULL;
}


inline void GRWLock::BeginReadLock()
{
	if (checkDeadlock()) return;
	AcquireSRWLockShared(&SRWLockHandle);
}


inline void GRWLock::EndReadLock()
{
	ReleaseSRWLockShared(&SRWLockHandle);
	LastThread = NULL;
}