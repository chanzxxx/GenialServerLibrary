// TestClient.cpp : Defines the entry point for the console application.
//

#include "../GenialServerLibrary/Network/GNetworkManager.h"
#include <conio.h>

class CTestClientEvent : public GEventBinder
{
	void onConnect(GSession& Session)
	{
		Session.Send_Sync((byte*)"Hello world", 12);
		printf("º¸³¿");
	}

} TestClientEvent;

int main()
{
	GNetworkManager* NetMan= GNetworkManager::CreateInstance();
	for (int i = 0; i < 1; i++)
	{
		GConnector* pConnector = NetMan->ConnectTo("127.0.0.1", 4889, TestClientEvent);
	}
	_getch();
    return 0;
}

