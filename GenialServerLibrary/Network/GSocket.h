#pragma once

#include "../Utility/GRWLock.h"
#include "GNetworkGlobal.h"

class GEventBinder;

enum IOType
{
	IO_Recv, // Accept, Connect ������ Recv�� ����. WSARecv�� �翬�� ����
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
	WSAOVERLAPPED	OverlappedIoData;	// Overlapped IO �����͸� ���� ����ü
	GSocket*		GSocket;			// Socket �����Ϳ� �����ϱ� ���� �����͸� ����
	WSABUF			InternalBuffer;		// �񵿱� I/O�� ����� ����
	IOType			IOType;		// ���� I/O �߿� � �۾�������
};

/**
*	@brief	Socket�� Overlapped ������ Wrapping �ϱ� ���� Ŭ�����Դϴ�.
*	@author	������(whdcksz@gmail.com)
*/
class GSocket : public GRWLock
{
public:
	
	/**
	*	@brief	���ο� ������ �����մϴ�
	*	@note	���� ������ �����Ұ�� SocketFailure�� throw �մϴ�.
	*/
	GSocket();

	virtual ~GSocket();

	virtual void Close();

	inline SOCKET getSocket() { return mSocket; }
	inline void setSocket(SOCKET Socket) { mSocket = Socket; }

	/**
	*	@brief	IOCP ���� ������ �����Ϳ� ���̸� �����մϴ�.
	*	@param	���ο� ������ �ּ�
	*	@param	���ο� ������ ����
	*/
	virtual void setSendIOCPBuffer(byte* Buffer, int Length)
	{
		SendIOCPContext.InternalBuffer.buf = (char*)Buffer;
		SendIOCPContext.InternalBuffer.len = Length;
	}

	/**
	*	@brief	IOCP ���� ������ �����Ϳ� ���̸� �����մϴ�.
	*	@param	���ο� ������ �ּ�
	*	@param	���ο� ������ ����
	*/
	virtual void setRecvIOCPBuffer(byte* Buffer, int Length)
	{
		RecvIOCPContext.InternalBuffer.buf = (char*)Buffer;
		RecvIOCPContext.InternalBuffer.len = Length;
	}

	/**
	*	@brief	���ӿ� ����ȭ�� ���� �ɼ��� �����մϴ�.
	*	@todo	SO_RCVBUF�� SO_SNDBUF �ɼ� ������ ���� �� ���� ������ �ʿ�(�����׽�Ʈ�غ���)
	*/
	virtual void setSocketOptions();

	/**
	*	@brief	Ŭ���̾�Ʈ ���� �����, Ŭ������ delete ���� �ʰ� ���� �ϱ� ���� �Լ��Դϴ� 
	*	@note	�������̵� �ϴ� Ŭ�������� �ݵ�� ȣ�����־���մϴ�.
	*	@todo	���� ���۾�
	*/
	virtual void CleanupForReuse()
	{
		closesocket(getSocket());
		//SendIOCPContext.OverlappedIoData
	}

	/**
	*	@brief		IOCP�� �� ������ �����մϴ�.
	*	@details	���� ������ ���۴� RecvBuffer�� ��ϵ˴ϴ�.
	*	@details	IOCP�� ����� �����ϸ� Exception�� �߻��մϴ�.
	*	@params		IOCP Handle
	*	@return		Success or not
	*/
	bool AttachToIOCP(HANDLE IOCPHandle);

	/**
	*	@brief		GQCS ���Ͻ� Io Context Type�� ���� �����մϴ�.
	*	@note		�� �Լ��� Listener, Connector, Session������ �ݵ�� Overriding �Ǿ�� ��.
	*	@return		���ϰ��� false �� ��� �� ������ �����˴ϴ�.
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

