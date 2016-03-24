#pragma once

#include "GEventBinder.h"
#include "GSession.h"
#include "GNetworkGlobal.h"

/**
*	@brief		분산서버를 위해 다른 서버에 연결 요청을 할 때 사용합니다.
*	@author		박종찬(whdcksz@gmail.com)
*	@todo		비동기 작업(ConnectEx?)으로 변환
*/
class GConnector :
	public GSession
{
public:
	GConnector(std::string &RemoteIPAddr_Str, unsigned short RemotePort);
	GConnector();
	virtual ~GConnector();

	/*
	* ConnectTo
	*	@param	RemoteIP: IP address of remote
	*	@param	RemotePort: Port number of remote address
	*	@return	true if success, false if fail
	*	@todo	Asynchronous connect
	*/
	void ConnectTo(unsigned int RemoteIPAddr, unsigned short RemotePort);
	bool IsConnected();

	void onConnected();


private:
	static unsigned int ConnectThread(void* Instance);
	unsigned int ConnectProc();
	bool bConnected;

	
	HANDLE mHandle;

};

