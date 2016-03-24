#pragma once

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")


#define _WINSOCK_DEPRECATED_NO_WARNINGS // 흠. inet_addr을 쓰기위한 임시.



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




// 64비트와 32비트 호환을 위한 타입정의
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
*	@brief	CreateServerWithParam 함수에 사용되는 구조체입니다.
*	@note	bUseVerifyPacket/VerifyPacketMax는 삭제 예정임
*	@todo	위와 동일
*/
struct CreateServerParams
{
	unsigned int IPAddress;		// 서버 아이피 주소
	unsigned short IPPort;		// 서버 포트번호
	int BacklogSize;			// Listen 시에 사용되는 backlog 숫자. AcceptEx를 위한 소켓풀을 만들때도 사용됨.

	int MaxPacketLength; // 이 이상 크기의 패킷이 들어오면 접속을 끊어버립니다 
	int BufferLength; // Send/Recv 버퍼의 크기
	int MaxConnection; // 최대 연결수
	uint64 AcceptTimeout; // Accept 이후 AcceptTimeout ms만큼 아무 응답이 없을시 연결을 끊습니다.

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
