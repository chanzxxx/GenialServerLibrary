#pragma once

#include "GEventBinder.h"
#include "GSession.h"
#include "GNetworkGlobal.h"

/**
*	@brief		�л꼭���� ���� �ٸ� ������ ���� ��û�� �� �� ����մϴ�.
*	@author		������(whdcksz@gmail.com)
*	@todo		�񵿱� �۾�(ConnectEx?)���� ��ȯ
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

