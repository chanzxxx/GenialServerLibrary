// GenialServerLibrary.cpp : Defines the entry point for the console application.
//

#include <conio.h>

#include "../GenialServerLibrary/Network/GNetworkManager.h"
#include "../GenialServerLibrary/Utility/GLogger.h"
#include "../GenialServerLibrary/Utility/GTimer.h"

class CEchoServerEvent : public GEventBinder
{
	onRecvReturnValue onRecv(GSession& Session)
	{
		GLogger::Log("EchoServer", "¹ÞÀ½: %s", Session.getRecvBuffer().getBufferPointer());
		Session.getSendBuffer().Write("Hi", 3);
		Session.StartSend();
		return onRecvReturnValue::Successful;
	}

} EchoServerEvent;

int main()
{
	GNetworkManager* NetMan = GNetworkManager::CreateInstance();
	GTimerManager* Timer = GTimerManager::getInstance();
	
	NetMan->Initiate();

	
	CreateServerParams Params;
	Params.BacklogSize = 128;
	Params.IPAddress = 0;
	Params.IPPort = htons(4889);
	Params.AcceptTimeout = 10000;

	NetMan->OpenPort(Params, EchoServerEvent);
	NetMan->MainProcess();
	NetMan->Shutdown();

	_getch();
	return 0;
}

