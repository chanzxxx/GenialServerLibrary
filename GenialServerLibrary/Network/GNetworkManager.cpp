#include "GNetworkManager.h"
#include "../Utility/GLogger.h"
#include "GSocket.h"
#include "../Utility/GTimer.h"

// Static ���� �ʱ�ȭ
GRWLock GLogger::ConsoleLock;
GNetworkManager* GNetworkManager::pInstance = NULL;

unsigned int WorkerThread(GNetworkManager* pNetWorkManager)
{
	//GLogger::Log("WorkerThread", "����Ǿ���!");
	return pNetWorkManager->WorkerThreadProcess(pNetWorkManager->getCPHandle());
}

GNetworkManager::GNetworkManager()
{
	if (!InitiateWinsock())
		throw WinsockInitiateFailure;

	pInstance = this;
}

GNetworkManager::~GNetworkManager()
{
}

void GNetworkManager::Initiate(int WorkerThreadCount)
{
	GTimerManager::CreateInstance();
	InitiateWinsock();
	CreateIOCP();
	CreateWorkerThreads(WorkerThreadCount);
}

GConnector* GNetworkManager::ConnectTo(std::string RemoteIP, unsigned short RemotePort, GEventBinder& Event)
{
	GConnector *newCon = new GConnector;
	unsigned int RemoteIPAddr = inet_addr(RemoteIP.c_str());
	RemoteIPAddr = ntohl(RemoteIPAddr);
	newCon->setEventBinder(&Event);
	newCon->ConnectTo(RemoteIPAddr, RemotePort);

	return newCon;
}

bool GNetworkManager::OpenPort(CreateServerParams& Params, GEventBinder& Event)
{
	GListener* newListener;
	try
	{
		newListener = new GListener;
	}
	catch (GNetworkErrorCode ErrCode)
	{
		printf("[GNetworkManager] OpenPort(): Critical Error! WSASocket() failed. WSAGetLastError: %d\n", WSAGetLastError());
		return false;
	}
	
	AddSocket(newListener);
	newListener->BindEvent(Event);
	newListener->MakeListen(Params, CompletionPortHandle);
	return true;
}

bool GNetworkManager::CreateServer(unsigned short Port)
{
	
	
	
}

void GNetworkManager::CreateWorkerThreads(int HowMany)
{
	HANDLE ThreadHandle;
	if (HowMany == 0)
	{
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);
		HowMany = SysInfo.dwNumberOfProcessors * 2;
	}
	try
	{
		// ��Ŀ������ ����
		for (int i = 0; i < HowMany; i++)
		{
			ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkerThread, (LPVOID)this, 0, NULL);
			if (ThreadHandle == INVALID_HANDLE_VALUE)
				throw GNetworkErrorCode::CreateThreadFailure;

			WorkerHandle[i] = ThreadHandle; // Save handle for further control
		}
		NumberOfWorkerThread = HowMany;
	}
	catch (GNetworkErrorCode Errcode)
	{
		GLogger::Log("GNetworkManager", "CreateServerWithParams() error - Windows CreateThread() failed\n");
		bRunning = false;
		return;
	}
	
}

void GNetworkManager::CreateIOCP()
{
	// Step1. IO Completion Port ����
	CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (CompletionPortHandle == NULL)
	{
		GLogger::Log("GNetworkManager", "CreateIOCP() : Completion port ���� ���� �����ڵ�: %d", GetLastError());
		return;
	}
	bRunning = true;
}

bool GNetworkManager::CreateServerWithParams(CreateServerParams& Param, GEventBinder& Event)
{
	// Step2. ���� ���ε�, ����, AcceptEx ó��. - OpenPort()���� ó��
	return OpenPort(Param, Event);
}

void GNetworkManager::Shutdown()
{
	bRunning = false;

	// ��Ŀ������� ����
	for (int i = 0; i < NumberOfWorkerThread; i++)
	{
		TerminateThread(WorkerHandle[i], 0);
	}

	// Ÿ�̸� �Ŵ��� ����
	GTimerManager::getInstance()->Shutdown();


	// ��� ������ ���������ϰ� ����
	for (std::map<SOCKET, GSocket*>::iterator it = SocketMap.begin(); it != SocketMap.end(); ++it)
	{
		it->second->Close();
		delete(it->second);
	}

	delete this;
}

bool GNetworkManager::MainProcess()
{
	while (bRunning)
	{
		GTimerManager::getInstance()->Proc();
	}
	return false;
}

void GNetworkManager::AddSocket(GSocket* SocketWrapper)
{
	SocketMapLock.BeginWriteLock();
	SocketMap.insert(std::pair<SOCKET, GSocket*>(SocketWrapper->getSocket(), SocketWrapper));
	SocketMapLock.EndWriteLock();
}

void GNetworkManager::DeleteSocket(SOCKET Socket)
{
	SocketMapLock.BeginWriteLock();
	SocketMap.erase(Socket);
	SocketMapLock.EndWriteLock();
}

void GNetworkManager::DeleteSocket(GSocket* SocketWrapper)
{
	SocketMapLock.BeginWriteLock();
	SocketMap.erase(SocketWrapper->getSocket());
	SocketMapLock.EndWriteLock();
}

GSocket* GNetworkManager::CreateSocket()
{
	GSocket* NewSocket;
	try
	{
		NewSocket = new GSocket;
	}
	catch (GNetworkErrorCode ErrCode)
	{
		// ���ο� ������ ����µ� ��������,
		// ���� �α� ����� exception �߻���Ű��.
		GLogger::Log("GNetworkManager", "CreateSocket() Error - WSASocket failure. Winsock ErrorCode %d\n", WSAGetLastError());
		return 0;
	}

	AddSocket(NewSocket);
	return NewSocket;
}

void GNetworkManager::CloseSocket(GSocket* Socket)
{
	Socket->Close();
	DeleteSocket(Socket);
}

bool GNetworkManager::InitiateWinsock()
{
	WSAData wsaData;
	int Result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Result != NO_ERROR)
	{
		GLogger::Log("GNetworkManager", "Winsock initiate failed with error: %d\n", Result);
		return false;
	}

	return true;
}

unsigned int GNetworkManager::WorkerThreadProcess(void* pCompletionPort)
{
	HANDLE CompletionPortHandle = (HANDLE)pCompletionPort;
	DWORD BytesTransferred;
	BOOL bRet;
	LPOVERLAPPED lpOverlapped;
	GSession *pSession;
	GIOCPContext* pIOData;

	while (bRunning)
	{
		pSession = NULL;
		pIOData = NULL;
		BytesTransferred = 0;

		/*
		GQCS ���� ��(���ϰ��� false) msdn ���� ����
		case 1. lpOverlapped�� NULL, GetLastError()�� ERROR_ABANDONED_WAIT_0 �� ���,
		==> �̹� ����� ���Ͽ� �۾��� ��
		==> ���� ���� CancelIo�� ���ֹǷ� �߻������� �����ʳ�..?
		
		case 2. lpOverlapped�� NULL�ε� GetLastError()�� ERROR_ABANDONED_WAIN_0�� �ƴ� ���,
		==> BytesTransferred, CompletionKey�� �����Ⱚ�� ����.
		
		case 3. lpOverlapped�� NULL�� �ƴ� ���.
		==>BytesTransferred, CompletionKey ���� ����� �µ�. ������ GetLastError() �� üũ�� ��
		*/

		bRet = GetQueuedCompletionStatus(CompletionPortHandle, &BytesTransferred, (PULONG_PTR)&pSession, &lpOverlapped, INFINITE);
		GLogger::Log("GNetworkManager", "GQCS %d %d %d", pSession, lpOverlapped, BytesTransferred);
		// ���н� ���� �ڵ�
		if (bRet==false)
		{
			DWORD Err = GetLastError();
			if (lpOverlapped == NULL) 
			{
				if (Err == ERROR_ABANDONED_WAIT_0) // case 1: ����� �ڵ鿡 I/o �۾��� ��û��.
				{
					// ���� ó���� �ʿ����. �̹� �����Ǿ��� ��
					GLogger::Log("GNetworkManager", "WorkerThreadProcess(): GQCS returned with error ERROR_ABANDONED_WAIT_0");
				}
				else // case 2: �װ� �ƴϸ� ���� �������� �α׳�����
				{
					GLogger::Log("GNetworkManager", "WorkerThreadProcess(): GQCS returned with unhandled error: %d", Err);
				}
			}
			else
			{
				if (pSession) // case 3: ���⼱ CompletionKey ������ pSession�� �Ѿ���־�� ����
				{
					if (Err == ERROR_NETNAME_DELETED) // �߰� case: ������ Hard close �� ��Ȳ. ����: http://javawork.egloos.com/2265358
					{
						pSession->Close();
						DeleteSocket(pSession);
					}
					// �װ͵� �ƴϸ�?
					// todo: ping-pong�� ���� alive üũ?
					else 
					{
						GLogger::Log("GNetworkManager", "WorkerThreadProcess(): GQCS returned with unhandled error_2: %d", Err);
					}
				}
			}
		}
		else // case 4: i/o, ������
		{
			
			pIOData = (GIOCPContext*)lpOverlapped;
			GLogger::Log("GNetworkManager", "GQCS %d %d %d", pSession->getSocket(), pIOData->GSocket->getSocket(), BytesTransferred);
			pSession->BeginWriteLock();
			if (pSession->onIoCompletion(*pIOData, BytesTransferred) == false)
			{
				pSession->Close();
				DeleteSocket(pSession);
			}
			pSession->EndWriteLock();
		}
	}
	
	GLogger::Log("GNetworkManager", "��Ŀ ������ ����");
	return 0;
}

void GNetworkManager::RegisterSocket(GSocket* SocketWrapper)
{
	AddSocket(SocketWrapper);
}

void GNetworkManager::TerminateSession(GSession& Session)
{
	Session.Close();
	DeleteSocket(&Session);
	delete &Session;
}

unsigned int GNetworkManager::getHostIPAddress()
{
	char IPStr[256];
	if (!gethostname(IPStr, 256))
	{
		printf("ip:%s\n", IPStr);
		return inet_addr(IPStr);
	}
	
	// �ƴϸ� ����
	GLogger::Log("GNetworkManager", "getHostIPAddress() fail, wsa errcode:%d", WSAGetLastError());
	return 0;
	
}