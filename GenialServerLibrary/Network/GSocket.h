#pragma once

#include "../Utility/GRWLock.h"
#include "GNetworkGlobal.h"

class GEventBinder;

enum IOType
{
	IO_Recv, // Accept, Connect 과정도 Recv로 간주. WSARecv는 당연히 포함
	IO_Send  // WSASend
};

enum GSocketType
{
	BasicSocket = 0,
	Session = 1,
	Listener = 2,
	Connector = 3,
	Acceptor = 4
};

class GSocket;

struct GIOCPContext
{
	WSAOVERLAPPED	OverlappedIoData;	// Overlapped IO 데이터를 닮은 구조체
	GSocket*		GSocket;			// Socket 데이터에 접근하기 위한 포인터를 담음
	WSABUF			InternalBuffer;		// 비동기 I/O시 사용할 버퍼
	IOType			IOType;		// 현재 I/O 중에 어떤 작업중인지
};

/**
*	@brief	Socket과 Overlapped 정보를 Wrapping 하기 위한 클래스입니다.
*	@author	박종찬(whdcksz@gmail.com)
*/
class GSocket : public GRWLock
{
public:
	
	/**
	*	@brief	새로운 소켓을 생성합니다
	*	@note	소켓 생성에 실패할경우 SocketFailure를 throw 합니다.
	*/
	GSocket();

	virtual ~GSocket();

	virtual void Close();

	inline SOCKET getSocket() { return mSocket; }
	inline void setSocket(SOCKET Socket) { mSocket = Socket; }

	/**
	*	@brief	IOCP 전송 버퍼의 포인터와 길이를 변경합니다.
	*	@param	새로운 버퍼의 주소
	*	@param	새로운 버퍼의 길이
	*/
	virtual void setSendIOCPBuffer(byte* Buffer, int Length)
	{
		SendIOCPContext.InternalBuffer.buf = (char*)Buffer;
		SendIOCPContext.InternalBuffer.len = Length;
	}

	/**
	*	@brief	IOCP 수신 버퍼의 포인터와 길이를 변경합니다.
	*	@param	새로운 버퍼의 주소
	*	@param	새로운 버퍼의 길이
	*/
	virtual void setRecvIOCPBuffer(byte* Buffer, int Length)
	{
		RecvIOCPContext.InternalBuffer.buf = (char*)Buffer;
		RecvIOCPContext.InternalBuffer.len = Length;
	}

	/**
	*	@brief	윈속에 최적화된 소켓 옵션을 적용합니다.
	*	@todo	SO_RCVBUF와 SO_SNDBUF 옵션 설정에 대해 더 많은 정보가 필요(직접테스트해보자)
	*/
	virtual void setSocketOptions();

	/**
	*	@brief	클라이언트 접속 종료시, 클래스를 delete 하지 않고 재사용 하기 위한 함수입니다 
	*	@note	오버라이딩 하는 클래스에서 반드시 호출해주어야합니다.
	*	@todo	아직 미작업
	*/
	virtual void CleanupForReuse()
	{
		closesocket(getSocket());
		//SendIOCPContext.OverlappedIoData
	}

	/**
	*	@brief		IOCP에 이 세션을 연결합니다.
	*	@details	최초 생성시 버퍼는 RecvBuffer로 등록됩니다.
	*	@details	IOCP에 등록을 실패하면 Exception이 발생합니다.
	*	@params		IOCP Handle
	*	@return		Success or not
	*/
	bool AttachToIOCP(HANDLE IOCPHandle);

	/**
	*	@brief		GQCS 리턴시 Io Context Type에 따라 실행합니다.
	*	@note		이 함수는 Listener, Connector, Session에서는 반드시 Overriding 되어야 함.
	*	@return		리턴값이 false 일 경우 이 세션은 삭제됩니다.
	*/
	virtual bool onIoCompletion(GIOCPContext& Context, unsigned int BytesTransferred);

	GIOCPContext& getRecvIOCPContext() { return RecvIOCPContext; }
	GSocketType getSocketType() { return SocketType; }
	
	void setEventBinder(GEventBinder* pEventBinder);

private:
	SOCKET mSocket;


protected:
	GSocketType SocketType;
	GIOCPContext SendIOCPContext;
	GIOCPContext RecvIOCPContext;
	GEventBinder* EventBinder;
	bool bEof;
};

