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
	// to do: ����� üũ �κ� �����Ƚ�
	// note: ���ι̻��� ���� ����. �ϴ� �ּ�ó��
/*	HANDLE newThread = GetCurrentThread();
	//printf("new:%d, last:%d\n", newThread, LastThread);
	if (newThread == LastThread)
	{
		GLogger::Log("GRWLock", "DeadLock �߰�! �ϴ� �ߺ� ���� Skip�մϴ�");
		return true;
	}
	LastThread = newThread;
	*/
	return false;
}