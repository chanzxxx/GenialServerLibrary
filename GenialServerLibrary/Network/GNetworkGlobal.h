#pragma once

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")


#define _WINSOCK_DEPRECATED_NO_WARNINGS // ��. inet_addr�� �������� �ӽ�.



//#include <MSWSock.h>


#define Default_MaxConnection 1024
#define Default_BufferSize	4096
#define DEFAULT_BACKLOGSIZE 1024
#define DEFAULT_MAXPACKETLENGTH 65536

enum GNetworkErrorCode
{
	WinsockInitiateFailure,
	ConnectFailure,
	ListenFailure,
	SocketFailure,
	MemoryFailure,
	CreateThreadFailure,
	IOCPFailure
};




// 64��Ʈ�� 32��Ʈ ȣȯ�� ���� Ÿ������
#ifdef _WIN64

//typedef	char	byte;
typedef char	int8;
typedef	short	int16;
typedef	int		int32;
typedef long	int64;
typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
typedef unsigned long	uint64;

#else


#endif

/**
*	@brief	CreateServerWithParam �Լ��� ���Ǵ� ����ü�Դϴ�.
*	@note	bUseVerifyPacket/VerifyPacketMax�� ���� ������
*	@todo	���� ����
*/
struct CreateServerParams
{
	unsigned int IPAddress;		// ���� ������ �ּ�
	unsigned short IPPort;		// ���� ��Ʈ��ȣ
	int BacklogSize;			// Listen �ÿ� ���Ǵ� backlog ����. AcceptEx�� ���� ����Ǯ�� ���鶧�� ����.

	int MaxPacketLength; // �� �̻� ũ���� ��Ŷ�� ������ ������ ��������ϴ� 
	int BufferLength; // Send/Recv ������ ũ��
	int MaxConnection; // �ִ� �����
	uint64 AcceptTimeout; // Accept ���� AcceptTimeout ms��ŭ �ƹ� ������ ������ ������ �����ϴ�.

	CreateServerParams()
		: IPAddress(0), IPPort(4889), BacklogSize(DEFAULT_BACKLOGSIZE), MaxPacketLength(DEFAULT_MAXPACKETLENGTH),
		BufferLength(Default_BufferSize), MaxConnection(Default_MaxConnection), AcceptTimeout(1000)
	{

	}
};


#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#include <stdio.h>
#include <atomic>
#include <cstdarg>
#include <memory.h>
#include <malloc.h>
#include <string>
#include <memory.h>
