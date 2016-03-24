#include "GSocket.h"
#include "../Utility/GLogger.h"
#include "GEventBinder.h"

GSocket::GSocket()
{
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mSocket == INVALID_SOCKET)
	{
		printf("[GSocket] Socket() failed errcode (%d)\n", WSAGetLastError());
		throw GNetworkErrorCode::SocketFailure;
	}
	SocketType = BasicSocket;
	memset(&RecvIOCPContext.OverlappedIoData, 0, sizeof(RecvIOCPContext.OverlappedIoData));
	memset(&SendIOCPContext.OverlappedIoData, 0, sizeof(SendIOCPContext.OverlappedIoData));
}


GSocket::~GSocket()
{
	

}

void GSocket::Close()
{
	// �������� I/o �۾��� �ִٸ� ����մϴ�.
	CancelIo((HANDLE)mSocket);

	// ������ �ݽ��ϴ�.
	if (mSocket) closesocket(mSocket);
}

void GSocket::setSocketOptions()
{
	// Linger �ɼ��� ���ϴ�.
	linger LingerOptions;
	LingerOptions.l_linger = 0;
	LingerOptions.l_onoff = 0;
	setsockopt(getSocket(), SOL_SOCKET, SO_LINGER, (const char*)&LingerOptions, sizeof(LingerOptions));

	// ���� ���۸� ���ִ� ����.
	// �ʿ���ٴ� ���� �־ �ϴ� ����.
	// https://groups.google.com/forum/#!topic/microsoft.public.platformsdk.networking/0RZYJpWLKnw
	/*
	DWORD dwSize = 0;
	setsockopt(getSocket(), SOL_SOCKET, SO_RCVBUF, (const char *)&dwSize, sizeof(dwSize));
	setsockopt(getSocket(), SOL_SOCKET, SO_SNDBUF, (const char *)&dwSize, sizeof(dwSize));
	*/
	bool Off = false;
	setsockopt(getSocket(), SOL_SOCKET, TCP_NODELAY, (const char*)&Off, sizeof(Off));

}

bool GSocket::AttachToIOCP(HANDLE IOCPHandle)
{
	// �ʱ�ȭ
	RecvIOCPContext.IOType = IO_Recv;
	SendIOCPContext.IOType = IO_Send;
	RecvIOCPContext.GSocket = this;
	SendIOCPContext.GSocket = this;

	HANDLE RetHandle = CreateIoCompletionPort((HANDLE)getSocket(), IOCPHandle, (ULONG_PTR)this, 0);
	if (RetHandle == NULL)
	{
		GLogger::Log("GSession", "AttachToIOCP() error - CreateIoCompletionPort() failed : errcode:%d\n", GetLastError());
		throw GNetworkErrorCode::IOCPFailure;
		return false;
	}

	return true;
}

bool GSocket::onIoCompletion(GIOCPContext& Context, unsigned int BytesTransferred)
{
	// ��ǻ� abstract �Լ��̹Ƿ� ȣ��Ǽ� �ȵȴ�.
	GLogger::Log("GSocket", "UNKNOWN ERROR: onIoCompletion() called by GSocket");
	return false; 
}


void GSocket::setEventBinder(GEventBinder* pEventBinder)
{
	EventBinder = pEventBinder;
}