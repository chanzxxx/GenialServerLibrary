#pragma once
#include "GSocket.h"
#include "GNetworkGlobal.h"
#include "GEventBinder.h"

class GListener:
	public GSocket
{
public:
	GListener();
	~GListener();

	bool MakeListen(CreateServerParams& Params, HANDLE IOCPHandle);
	
	virtual bool onIoCompletion(GIOCPContext& Context, unsigned int BytesTransferred) override;
	bool CheckAcceptorPool();
	void setOptions(CreateServerParams& Param);
	void BindEvent(GEventBinder& Event);
private:
	int AcceptorPoolCount;
	HANDLE mIOCPHandle;
	CreateServerParams Options;
	
	void CreateAcceptorPool();
	
};

