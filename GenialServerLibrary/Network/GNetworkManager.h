#pragma once

#include <map>

#include "GEventBinder.h"
#include "GConnector.h"
#include "GListener.h"
#include "GNetworkGlobal.h"
#include "../Utility/GRWLock.h"
#include "GEventBinder.h"


#define MAX_WORKERTHREAD 32



/**
*	@brief ��Ʈ��ũ �Ŵ��� Ŭ����. ��ü ��Ʈ��ũ ������ ���.
*	@author ������(whdcksz@gmail.com)
*/
class GNetworkManager
{
public:
	
	static inline GNetworkManager* CreateInstance();
	static inline GNetworkManager* getInstance();
	void Initiate(int WorkerThreadCount=0);
	
	/**
	*	@brief ��� �������� ����õ�.
	*	@param ������ ������ �ּ�
	*	@param ������ ��Ʈ
	*	@return ������ GConnector�� ������, ���н� NULL
	*/
	GConnector* ConnectTo(std::string RemoteIP, unsigned short RemotePort, GEventBinder& Event = EventBinderDefault);


	/**
	*	@brief ������ �޾Ƶ��̴� ������ ����.
	*	@param CreateServerParams
	*	@return ���� ����
	*/
	bool OpenPort(CreateServerParams& Params, GEventBinder& Event = EventBinderDefault);


	/**
	*	@brief ���� �Լ����� �ֱ������� ȣ��Ǿ�� �ϴ� ���η��� �Լ�
	*/
	bool MainProcess();


	/**
	*	@brief	�⺻ �ɼ��� �����ؼ� ������ ���ϴ�
	*	@params	��Ʈ�� �������ݴϴ�.
	*	@return ���� ����
	*	@see	CreateServerWithParams
	*/
	bool CreateServer(unsigned short Port);


	/**
	*	@brief	������ ���ϴ�.
	*	@params	CreateServerParams - ������ �������� �ʿ��� ����
	*	@return ���� ����
	*	@see	struct CreateServerParams
	*/
	bool CreateServerWithParams(CreateServerParams& Param, GEventBinder& Event = EventBinderDefault);
	
	/**
	*	@brief	������ �����մϴ�.
	*	@details ��� ����� ��ü�� ��Ŀ �����带 �����մϴ�.
	*/
	void Shutdown();

	void RegisterSocket(GSocket*);

	void TerminateSession(GSession&);

	/**
	*	@brief	��Ŀ�����忡�� ȣ��Ǵ� �Լ�
	*	@params	IOCP Handle
	*	@return �����尡 ��������Ǹ� 0, �̿��� ���� ������ �Բ� ����.
	*	@see	WorkerThread()
	*/
	unsigned int WorkerThreadProcess(void* pCompletionPort);

	unsigned int getHostIPAddress();
	HANDLE getCPHandle() { return CompletionPortHandle; }

	bool getbRunning() { return bRunning; }

private:
	std::map<SOCKET, GSocket*> SocketMap;
	GRWLock SocketMapLock; // Map�� Insert �ϴ� �۾��� thread-safe���� �����Ƿ� ���� �۾����� Lock ó���� ����.

	HANDLE CompletionPortHandle;
	int NumberOfWorkerThread;
	bool InitiateWinsock();
	void CreateWorkerThreads(int HowMany = 0);
	void CreateIOCP();
	HANDLE WorkerHandle[MAX_WORKERTHREAD];
	
	GSocket* CreateSocket();
	void CloseSocket(GSocket* Socket);
	void AddSocket(GSocket* SocketWrapper);
	void DeleteSocket(SOCKET Socket);
	void DeleteSocket(GSocket* SocketWrapper);

	bool bRunning;

	GNetworkManager();
	~GNetworkManager();
private:
	static GNetworkManager *pInstance; // Singleton
};

inline GNetworkManager* GNetworkManager::CreateInstance() { return pInstance = new GNetworkManager; }
inline GNetworkManager* GNetworkManager::getInstance() { return pInstance; }