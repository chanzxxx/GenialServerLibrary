#include "GConnector.h"
#include "../Utility/GLogger.h"
#include "GNetworkManager.h"
#include "../Utility/GRWLock.h"

GConnector::GConnector(std::string &RemoteIPAddr_Str, unsigned short RemotePort)
	: mHandle(NULL)
{
	unsigned int RemoteIPAddr = inet_addr(RemoteIPAddr_Str.c_str());
	RemoteIPAddr = ntohl(RemoteIPAddr);
	ConnectTo(RemoteIPAddr, RemotePort);
	SocketType = Connector;
}

GConnector::GConnector()
	: mHandle(NULL)
{
	SocketType = Connector;
}

GConnector::~GConnector()
{
	TerminateThread(mHandle, 0);
}

void GConnector::ConnectTo(unsigned int RemoteIPAddr, unsigned short RemotePort)
{
	mRemoteAddr = RemoteIPAddr;
	mRemotePort = RemotePort;

	mHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&GConnector::ConnectThread, this, 0, 0);
}

bool GConnector::IsConnected()
{
	return bConnected;
}


unsigned int GConnector::ConnectThread(void* pInstance)
{
	return ((GConnector*)pInstance)->ConnectProc();
}


unsigned int GConnector::ConnectProc()
{
	struct sockaddr_in RemoteAddr;
	RemoteAddr.sin_family = AF_INET;
	RemoteAddr.sin_port = htons(mRemotePort);
	RemoteAddr.sin_addr.s_addr = htonl(mRemoteAddr);

	int Result = connect(getSocket(), (SOCKADDR*)&RemoteAddr, sizeof(RemoteAddr));

	if (Result == SOCKET_ERROR)
	{
		printf("[GConnector] Connect fail: WinsockError[%ld]\n", WSAGetLastError());
		closesocket(getSocket());
		return false;
	}

	bConnected = true;
	onConnected();
	return 1;
}

void GConnector::onConnected()
{
	AttachToIOCP(GNetworkManager::getInstance()->getCPHandle());
	GNetworkManager::getInstance()->RegisterSocket(this);
	GLogger::Log("GConnector", "¿¬°áµÊ!");
	
	EventBinder->onConnect(*this);
}