#pragma once

#include "GSocket.h"
#include "GPacketBuffer.h"

class GEventBinder;

class GSession : public GSocket
{
public:
	GSession();
	virtual ~GSession();

	virtual void Close();

	GPacketBuffer& getRecvBuffer() { return *RecvBuffer; }
	GPacketBuffer& getSendBuffer() { return *SendBuffer; }

	void Send_Sync(byte* Data, int length);

	void WaitForAccept();
	void Activate();

	void* getSessionData() { return SessionData; }
	void setSessionData(void *Data) { SessionData = Data; }

	virtual bool onIoCompletion(GIOCPContext& Context, unsigned int BytesTransferred) override;

	virtual uint64 getlastContactTime()
	{
		return lastContactTime;
	}

	virtual void setlastContactTime(uint64 Time)
	{
		lastContactTime = Time;
	}
	
	void setRemoteAddr(uint32 Addr)
	{
		mRemoteAddr = Addr;
	}

	void setRemotePort(uint16 Port)
	{
		mRemotePort = Port;
	}
	


	bool StartRecv();
	bool StartSend();

	inline void AllocRecvBuffer(uint64 BufferSize);
	inline void AllocSendBuffer(uint64 BufferSize);

private:
	
	GPacketBuffer* SendBuffer;
	GPacketBuffer* RecvBuffer;
	void* SessionData;
	uint64 lastContactTime;

protected:
	uint32 mRemoteAddr;
	uint16 mRemotePort;
};

inline void GSession::AllocRecvBuffer(uint64 BufferSize)
{
	RecvBuffer = new GPacketBuffer(BufferSize);
}

inline void GSession::AllocSendBuffer(uint64 BufferSize)
{
	SendBuffer = new GPacketBuffer(BufferSize);
}