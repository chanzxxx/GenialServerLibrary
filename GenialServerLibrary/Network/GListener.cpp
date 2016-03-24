#include "GListener.h"
#include "GSession.h"
#include "../Utility/GLogger.h"
#include "GEventBinder.h"
#include "GNetworkManager.h"
#include "../Utility/GTimer.h"

GListener::GListener()
{
	SocketType = Listener;
}

GListener::~GListener()
{

}

bool GListener::MakeListen(CreateServerParams& Params, HANDLE IOCPHandle)
{
	int Ret;
	// listen 소켓의 옵션을 설정해준다.
	bool on = true;
	Ret = setsockopt(getSocket(), SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&on, sizeof(on));
	if (Ret == -1) // mmm.. how to handle this?
		GLogger::Log("GListener", "Warning: MakeListen() setsockopt failed - Winsock Errcode: %d\n", WSAGetLastError());

	// 소켓을 인자로 넘어온 주소에 바인드 하고 Listen 상태로 만든다.
	SOCKADDR_IN ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = Params.IPAddress;
	ServerAddr.sin_port = Params.IPPort;
	if (bind(getSocket(), (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		GLogger::Log("GListener", "MakeListen()-bind failed: WSAErrcode:%d", WSAGetLastError());
		return false;
	}
	if (listen(getSocket(), Params.BacklogSize) == SOCKET_ERROR)
	{
		GLogger::Log("GListener", "MakeListen()-listen failed: WSAErrcode:%d", WSAGetLastError());
		return false;
	}

	setOptions(Params);
	mIOCPHandle = IOCPHandle;
	AttachToIOCP(mIOCPHandle);
	CreateAcceptorPool();
	return true;
}

void GListener::CreateAcceptorPool()
{
	// Acceptor Pool에 메모리 할당
	AcceptorPoolCount = 0;

	DWORD dwBytesReceived; // NOTE: AcceptEx 함수에 주소값을 넘겨야 하는데, 지역변수로 넘기면 무슨 의미가 있지? 다른 코드를 보아도 특별한 의미는 없는듯 함.
	
	/*
	*	Note: Acceptor는 Session입니다. 단, 빈 Session의 소켓에 AcceptEx가 걸려있는, 접속 대기중인 상태입니다.
	*	메인스레드에서 GQCS를 통해 Accept가 완료되었음이 통지되면 상태값을 변경해 Session의 역할을 수행하게 됩니다.
	*/
	for (int i = 0; i < Options.BacklogSize; i++)
	{
		GSession* Acceptor = new GSession;

		// Recv 버퍼 할당; Send 버퍼는 아직 필요없으므로 Accept 된 이후에 할당해줍니다.
		Acceptor->AllocRecvBuffer(Options.BufferLength);

		if (AcceptEx(getSocket(), Acceptor->getSocket(), Acceptor->getRecvBuffer().getBufferPointer(), 0/*VerifyPacketMax*/, // 이 값을 0이 아닌값으로 설정하면 그 숫자만큼 데이터를 버퍼에 Recv함.
			(sizeof(sockaddr_in) + 16), (sizeof(sockaddr_in) + 16), &dwBytesReceived, &Acceptor->getRecvIOCPContext().OverlappedIoData) == FALSE)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				GLogger::Log("GListener", "AcceptEx Fail: %d", WSAGetLastError());
				continue;
			}
		}

		// 세션 상태를 Accept 대기 상태로 변경
		Acceptor->WaitForAccept();
		
		// Acceptor 세션데이터에 Listener 정보를 담아줌 (후에 Accept I/O 완료 통지가 오면 처리하기 위해) 
		Acceptor->setSessionData(this);
		
		// IOCP에 연결
		if (Acceptor->AttachToIOCP(mIOCPHandle))
			AcceptorPoolCount++;
	}
}

bool GListener::CheckAcceptorPool()
{
	if (AcceptorPoolCount == 0)
	{
		CreateAcceptorPool();
		return true;
	}
	return false;
}

void GListener::setOptions(CreateServerParams& Param)
{
	memcpy(&Options, &Param, sizeof(Param));
}

void GListener::BindEvent(GEventBinder & Event)
{
	EventBinder = &Event;
}

bool GListener::onIoCompletion(GIOCPContext& Context, unsigned int BytesTransferred)
{
	// 접속된 세션에 대해 유저 이벤트 호출
	GSession* Session = dynamic_cast<GSession*>(Context.GSocket);
	if (Session == NULL)
	{
		GLogger::Log("GListener", "onIoCompletion() - type cast failed");
		return true;
	}// ~?!
	
	if (EventBinder->onAccept(*Session) == GEventBinder::onAcceptReturnValue::Grant)
	{
		// Send 버퍼 할당
		Session->AllocSendBuffer(Options.BufferLength);
		// 
		Session->Activate();
		Session->setEventBinder(EventBinder);

		// 세션을 Network Manager에 등록
		GNetworkManager::getInstance()->RegisterSocket(Session);

		// 원격 주소 가져오기
		SOCKADDR* LocalAddr;
		int LocalAddrLen;
		SOCKADDR* RemoteAddr;
		int RemoteAddrLen;
		GetAcceptExSockaddrs(Session->getRecvBuffer().getBufferPointer(), BytesTransferred, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
			&LocalAddr, &LocalAddrLen, &RemoteAddr, &RemoteAddrLen);

		Session->setRemoteAddr(((SOCKADDR_IN*)LocalAddr)->sin_addr.s_addr);
		Session->setRemotePort(ntohs(((SOCKADDR_IN*)LocalAddr)->sin_port));
		Session->setlastContactTime(GTimerManager::getNowTime());

		// 세션을 Timer에 등록: 주기적으로 체크해서 일정시간 만큼 통신이 없는 세션은 짤라버립니다.
		GTimer* Timer = GTimerManager::getInstance()->CreateTimer();
		Timer->setData((uint64)Session);
		Timer->setSecondData((uint64)this);
		Timer->setInterval(Options.AcceptTimeout);
		Timer->setTickEvent([](GTimer& Timer) {
			GSession* Session = (GSession*)(Timer.getData());
			if (Session->getlastContactTime() + Timer.getInterval() <= GTimerManager::getNowTime())
				Session->Close();
			return false;
		});
		Timer->Activate();

		Session->StartRecv();
	}
	else
		Session->Close();

	AcceptorPoolCount--;
	//GLogger::Log("GListener", "APC: %d", AcceptorPoolCount);
	CheckAcceptorPool();
	
	return true;
}

