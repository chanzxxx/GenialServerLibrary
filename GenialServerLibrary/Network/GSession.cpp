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
		if (BytesTransferred == 0) // 연결 종료
			return false;

		else
		{
			// WSARecv를 통해 버퍼에 다이렉트로 수신받았기 때문에, 버퍼에 길이만 기록해줍니다.
			RecvBuffer->addDataLength(BytesTransferred);

			// Recv 이벤트를 호출합니다.
			GEventBinder::onRecvReturnValue Result = EventBinder->onRecv(*this);


			// 결과에 따라 처리합니다.
			// Successful인 경우 버퍼를 비우고 true를 리턴해 다음 Recv i/o를 시작합니다.
			if (Result == GEventBinder::onRecvReturnValue::Successful)
			{
				RecvBuffer->Flush();
				return true;
			}

			// Close인 경우 false를 리턴함으로써 워커쓰레드에서 세션을 삭제하도록 합니다.
			else if (Result == GEventBinder::onRecvReturnValue::Successful_Close ||
				Result == GEventBinder::onRecvReturnValue::Error_Close)
				return false;

			// ProccessOnNextRecv는 패킷이 아직다 도착하지 않았을 때를 위해 만들어진 옵션입니다.
			else if(Result == GEventBinder::onRecvReturnValue::ProcessOnNextRecv)
				return true;
		}
	}

	else if (Context.IOType == IO_Send) // Send 완료 통지
	{
		if (BytesTransferred < SendBuffer->getDataLength()) // 다안보내졌나? 이런 케이스가 발생하는가?
		{
			GLogger::Log("GSession", "onIoCompletion(): 패킷이 지정한 만큼 전송되지 않았습니다");
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