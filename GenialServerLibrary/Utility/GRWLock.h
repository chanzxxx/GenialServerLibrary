#pragma once
#include "../Network/GNetworkGlobal.h"

/**
*	@brief		�����ڿ��� multi-read, single write�� ���� SRW Lock�� Wrapper Ŭ���� �Դϴ�.
*	@todo		�ϴ� SRWLock�� ������ ����������Ƿ� ���� ������ ��~~~~~�߿� �ϴ°ɷ�..
*/
class GRWLock
{
public:
	GRWLock();

	virtual ~GRWLock();

	/**
	*	@brief	 �� �����忡�� WriteLock�� �����մϴ�.
	*/
	inline void BeginWriteLock();

	/**
	*	@brief	�� �������� WriteLock�� �����մϴ�.
	*/
	inline void EndWriteLock();

	/**
	*	@brief	 �� �����忡�� ReadLock�� �����մϴ�.
	*/
	inline void BeginReadLock();

	/**
	*	@brief	 �� �����忡�� ReadLock�� �����մϴ�.
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