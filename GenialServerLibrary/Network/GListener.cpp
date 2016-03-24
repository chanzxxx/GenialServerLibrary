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
	// listen ������ �ɼ��� �������ش�.
	bool on = true;
	Ret = setsockopt(getSocket(), SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&on, sizeof(on));
	if (Ret == -1) // mmm.. how to handle this?
		GLogger::Log("GListener", "Warning: MakeListen() setsockopt failed - Winsock Errcode: %d\n", WSAGetLastError());

	// ������ ���ڷ� �Ѿ�� �ּҿ� ���ε� �ϰ� Listen ���·� �����.
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
	// Acceptor Pool�� �޸� �Ҵ�
	AcceptorPoolCount = 0;

	DWORD dwBytesReceived; // NOTE: AcceptEx �Լ��� �ּҰ��� �Ѱܾ� �ϴµ�, ���������� �ѱ�� ���� �ǹ̰� ����? �ٸ� �ڵ带 ���Ƶ� Ư���� �ǹ̴� ���µ� ��.
	
	/*
	*	Note: Acceptor�� Session�Դϴ�. ��, �� Session�� ���Ͽ� AcceptEx�� �ɷ��ִ�, ���� ������� �����Դϴ�.
	*	���ν����忡�� GQCS�� ���� Accept�� �Ϸ�Ǿ����� �����Ǹ� ���°��� ������ Session�� ������ �����ϰ� �˴ϴ�.
	*/
	for (int i = 0; i < Options.BacklogSize; i++)
	{
		GSession* Acceptor = new GSession;

		// Recv ���� �Ҵ�; Send ���۴� ���� �ʿ�����Ƿ� Accept �� ���Ŀ� �Ҵ����ݴϴ�.
		Acceptor->AllocRecvBuffer(Options.BufferLength);

		if (AcceptEx(getSocket(), Acceptor->getSocket(), Acceptor->getRecvBuffer().getBufferPointer(), 0/*VerifyPacketMax*/, // �� ���� 0�� �ƴѰ����� �����ϸ� �� ���ڸ�ŭ �����͸� ���ۿ� Recv��.
			(sizeof(sockaddr_in) + 16), (sizeof(sockaddr_in) + 16), &dwBytesReceived, &Acceptor->getRecvIOCPContext().OverlappedIoData) == FALSE)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				GLogger::Log("GListener", "AcceptEx Fail: %d", WSAGetLastError());
				continue;
			}
		}

		// ���� ���¸� Accept ��� ���·� ����
		Acceptor->WaitForAccept();
		
		// Acceptor ���ǵ����Ϳ� Listener ������ ����� (�Ŀ� Accept I/O �Ϸ� ������ ���� ó���ϱ� ����) 
		Acceptor->setSessionData(this);
		
		// IOCP�� ����
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
	// ���ӵ� ���ǿ� ���� ���� �̺�Ʈ ȣ��
	GSession* Session = dynamic_cast<GSession*>(Context.GSocket);
	if (Session == NULL)
	{
		GLogger::Log("GListener", "onIoCompletion() - type cast failed");
		return true;
	}// ~?!
	
	if (EventBinder->onAccept(*Session) == GEventBinder::onAcceptReturnValue::Grant)
	{
		// Send ���� �Ҵ�
		Session->AllocSendBuffer(Options.BufferLength);
		// 
		Session->Activate();
		Session->setEventBinder(EventBinder);

		// ������ Network Manager�� ���
		GNetworkManager::getInstance()->RegisterSocket(Session);

		// ���� �ּ� ��������
		SOCKADDR* LocalAddr;
		int LocalAddrLen;
		SOCKADDR* RemoteAddr;
		int RemoteAddrLen;
		GetAcceptExSockaddrs(Session->getRecvBuffer().getBufferPointer(), BytesTransferred, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
			&LocalAddr, &LocalAddrLen, &RemoteAddr, &RemoteAddrLen);

		Session->setRemoteAddr(((SOCKADDR_IN*)LocalAddr)->sin_addr.s_addr);
		Session->setRemotePort(ntohs(((SOCKADDR_IN*)LocalAddr)->sin_port));
		Session->setlastContactTime(GTimerManager::getNowTime());

		// ������ Timer�� ���: �ֱ������� üũ�ؼ� �����ð� ��ŭ ����� ���� ������ ©������ϴ�.
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

