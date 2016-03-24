#include "GRWLock.h"
#include "GLogger.h"

GRWLock::GRWLock()
{
	InitializeSRWLock(&SRWLockHandle);
	LastThread = NULL;
}

GRWLock::~GRWLock()
{
}

bool GRWLock::checkDeadlock()
{
	// to do: 데드락 체크 부분 버그픽스
	// note: 원인미상의 버그 있음. 일단 주석처리
/*	HANDLE newThread = GetCurrentThread();
	//printf("new:%d, last:%d\n", newThread, LastThread);
	if (newThread == LastThread)
	{
		GLogger::Log("GRWLock", "DeadLock 발견! 일단 중복 락은 Skip합니다");
		return true;
	}
	LastThread = newThread;
	*/
	return false;
}