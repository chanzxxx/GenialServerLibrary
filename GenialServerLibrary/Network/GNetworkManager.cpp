#include "GNetworkManager.h"
#include "../Utility/GLogger.h"
#include "GSocket.h"
#include "../Utility/GTimer.h"

// Static 변수 초기화
GRWLock GLogger::ConsoleLock;
GNetworkManager* GNetworkManager::pInstance = NULL;

unsigned int WorkerThread(GNetworkManager* pNetWorkManager)
{
	//GLogger::Log("WorkerThread", "실행되었다!");
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
		// 워커쓰레드 생성
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
	// Step1. IO Completion Port 생성
	CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (CompletionPortHandle == NULL)
	{
		GLogger::Log("GNetworkManager", "CreateIOCP() : Completion port 생성 실패 에러코드: %d", GetLastError());
		return;
	}
	bRunning = true;
}

bool GNetworkManager::CreateServerWithParams(CreateServerParams& Param, GEventBinder& Event)
{
	// Step2. 소켓 바인드, 리슨, AcceptEx 처리. - OpenPort()에서 처리
	return OpenPort(Param, Event);
}

void GNetworkManager::Shutdown()
{
	bRunning = false;

	// 워커쓰레드들 종료
	for (int i = 0; i < NumberOfWorkerThread; i++)
	{
		TerminateThread(WorkerHandle[i], 0);
	}

	// 타이머 매니저 종료
	GTimerManager::getInstance()->Shutdown();


	// 모든 소켓을 접속종료하고 삭제
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
		// 새로운 소켓을 만드는데 실패했음,
		// 원인 로그 남기고 exception 발생시키자.
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
		GQCS 실패 시(리턴값이 false) msdn 설명 정리
		case 1. lpOverlapped가 NULL, GetLastError()가 ERROR_ABANDONED_WAIT_0 인 경우,
		==> 이미 종료된 소켓에 작업한 것
		==> 종료 전에 CancelIo를 해주므로 발생할일이 없지않나..?
		
		case 2. lpOverlapped가 NULL인데 GetLastError()가 ERROR_ABANDONED_WAIN_0이 아닌 경우,
		==> BytesTransferred, CompletionKey에 쓰레기값이 들어옴.
		
		case 3. lpOverlapped가 NULL이 아닌 경우.
		==>BytesTransferred, CompletionKey 값이 제대로 셋됨. 원인은 GetLastError() 로 체크할 것
		*/

		bRet = GetQueuedCompletionStatus(CompletionPortHandle, &BytesTransferred, (PULONG_PTR)&pSession, &lpOverlapped, INFINITE);
		GLogger::Log("GNetworkManager", "GQCS %d %d %d", pSession, lpOverlapped, BytesTransferred);
		// 실패시 에러 핸들
		if (bRet==false)
		{
			DWORD Err = GetLastError();
			if (lpOverlapped == NULL) 
			{
				if (Err == ERROR_ABANDONED_WAIT_0) // case 1: 종료된 핸들에 I/o 작업이 요청됨.
				{
					// 따로 처리가 필요없음. 이미 삭제되었을 것
					GLogger::Log("GNetworkManager", "WorkerThreadProcess(): GQCS returned with error ERROR_ABANDONED_WAIT_0");
				}
				else // case 2: 그게 아니면 무슨 에러인지 로그남기자
				{
					GLogger::Log("GNetworkManager", "WorkerThreadProcess(): GQCS returned with unhandled error: %d", Err);
				}
			}
			else
			{
				if (pSession) // case 3: 여기선 CompletionKey 값으로 pSession이 넘어와있어야 정상
				{
					if (Err == ERROR_NETNAME_DELETED) // 추가 case: 접속이 Hard close 된 상황. 참고: http://javawork.egloos.com/2265358
					{
						pSession->Close();
						DeleteSocket(pSession);
					}
					// 그것도 아니면?
					// todo: ping-pong을 통해 alive 체크?
					else 
					{
						GLogger::Log("GNetworkManager", "WorkerThreadProcess(): GQCS returned with unhandled error_2: %d", Err);
					}
				}
			}
		}
		else // case 4: i/o, 성공적
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
	
	GLogger::Log("GNetworkManager", "워커 쓰레드 종료");
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
	
	// 아니면 에러
	GLogger::Log("GNetworkManager", "getHostIPAddress() fail, wsa errcode:%d", WSAGetLastError());
	return 0;
	
}