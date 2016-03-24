#include "GSession.h"
#include "GEventBinder.h"
#include "../Utility/GLogger.h"
#include "../Utility/GTimer.h"

GSession::GSession()
{
	SocketType = Session;
}


GSession::~GSession()
{
}


void GSession::WaitForAccept()
{
	SocketType = GSocketType::Acceptor;
}

void GSession::Activate()
{
	SocketType = GSocketType::Session;
}

void GSession::Close()
{
	EventBinder->onClose(*this);
	struct in_addr Addr;
	Addr.s_addr = mRemoteAddr;
	GLogger::Log("GSession", "Close:%s", inet_ntoa(Addr));
	GSocket::Close();
}

bool GSession::onIoCompletion(GIOCPContext& Context, unsigned int BytesTransferred)
{
	setlastContactTime(GTimerManager::getNowTime());
	
	if (Context.IOType == IO_Recv)
	{
		if (BytesTransferred == 0) // ���� ����
			return false;

		else
		{
			// WSARecv�� ���� ���ۿ� ���̷�Ʈ�� ���Ź޾ұ� ������, ���ۿ� ���̸� ������ݴϴ�.
			RecvBuffer->addDataLength(BytesTransferred);

			// Recv �̺�Ʈ�� ȣ���մϴ�.
			GEventBinder::onRecvReturnValue Result = EventBinder->onRecv(*this);


			// ����� ���� ó���մϴ�.
			// Successful�� ��� ���۸� ���� true�� ������ ���� Recv i/o�� �����մϴ�.
			if (Result == GEventBinder::onRecvReturnValue::Successful)
			{
				RecvBuffer->Flush();
				return true;
			}

			// Close�� ��� false�� ���������ν� ��Ŀ�����忡�� ������ �����ϵ��� �մϴ�.
			else if (Result == GEventBinder::onRecvReturnValue::Successful_Close ||
				Result == GEventBinder::onRecvReturnValue::Error_Close)
				return false;

			// ProccessOnNextRecv�� ��Ŷ�� ������ �������� �ʾ��� ���� ���� ������� �ɼ��Դϴ�.
			else if(Result == GEventBinder::onRecvReturnValue::ProcessOnNextRecv)
				return true;
		}
	}

	else if (Context.IOType == IO_Send) // Send �Ϸ� ����
	{
		if (BytesTransferred < SendBuffer->getDataLength()) // �پȺ�������? �̷� ���̽��� �߻��ϴ°�?
		{
			GLogger::Log("GSession", "onIoCompletion(): ��Ŷ�� ������ ��ŭ ���۵��� �ʾҽ��ϴ�");
			StartSend();
		}
		else
			SendBuffer->Flush();

		return true;
	}

	return false; // ?
}



bool GSession::StartRecv()
{
	DWORD dwError = 0, dwFlags = 0;
	memset(&RecvIOCPContext.OverlappedIoData, 0, sizeof(RecvIOCPContext.OverlappedIoData));
	SendIOCPContext.IOType = IOType::IO_Recv;
	RecvIOCPContext.InternalBuffer.buf = (char*)RecvBuffer->getCurrentOffsetPointer();
	RecvIOCPContext.InternalBuffer.len = RecvBuffer->getRestSpace();

	int Result = WSARecv(getSocket(), &RecvIOCPContext.InternalBuffer, 1, NULL, &dwFlags, &RecvIOCPContext.OverlappedIoData, NULL);
	if (Result == SOCKET_ERROR)
	{
		DWORD Err = WSAGetLastError();
		if (Err != ERROR_IO_PENDING && dwError != ERROR_SUCCESS)
		{
			GLogger::Log("GSession", "WSARecv() failed with error : %d", Err);
			return false;
		}
	}
	return true;
}

bool GSession::StartSend()
{
	DWORD dwError = 0, dwFlags = 0;
	memset(&SendIOCPContext.OverlappedIoData, 0, sizeof(SendIOCPContext.OverlappedIoData));
	SendIOCPContext.IOType = IOType::IO_Send;
	SendIOCPContext.InternalBuffer.buf = (char*)SendBuffer->getCurrentOffsetPointer();
	SendIOCPContext.InternalBuffer.len = SendBuffer->getDataLength()/*- SendBuffer->getCurOffset() */;

	int Result = WSASend(getSocket(), &SendIOCPContext.InternalBuffer, 1, NULL, dwFlags, &SendIOCPContext.OverlappedIoData, NULL);
	if (Result == SOCKET_ERROR)
	{
		DWORD Err = WSAGetLastError();
		if (Err != ERROR_IO_PENDING && dwError != ERROR_SUCCESS)
		{
			GLogger::Log("GSession", "WSARecv() failed with error : %d", Err);
			return false;
		}
	}
	return true;
}

void GSession::Send_Sync(byte* Data, int length)
{
	int ret = send(getSocket(), (const char*)Data, length, 0);
	if (ret == SOCKET_ERROR)
		GLogger::Log("GSession", "Send_Sync() failed with winsock err:%d", WSAGetLastError());

}