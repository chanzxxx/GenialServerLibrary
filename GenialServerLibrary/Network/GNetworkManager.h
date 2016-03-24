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
*	@brief 네트워크 매니저 클래스. 전체 네트워크 관리를 담당.
*	@author 박종찬(whdcksz@gmail.com)
*/
class GNetworkManager
{
public:
	
	static inline GNetworkManager* CreateInstance();
	static inline GNetworkManager* getInstance();
	void Initiate(int WorkerThreadCount=0);
	
	/**
	*	@brief 대상 원격지에 연결시도.
	*	@param 원격지 아이피 주소
	*	@param 원격지 포트
	*	@return 성공시 GConnector의 포인터, 실패시 NULL
	*/
	GConnector* ConnectTo(std::string RemoteIP, unsigned short RemotePort, GEventBinder& Event = EventBinderDefault);


	/**
	*	@brief 접속을 받아들이는 소켓을 열음.
	*	@param CreateServerParams
	*	@return 성공 여부
	*/
	bool OpenPort(CreateServerParams& Params, GEventBinder& Event = EventBinderDefault);


	/**
	*	@brief 메인 함수에서 주기적으로 호출되어야 하는 메인루프 함수
	*/
	bool MainProcess();


	/**
	*	@brief	기본 옵션을 적용해서 서버를 엽니다
	*	@params	포트만 지정해줍니다.
	*	@return 성공 여부
	*	@see	CreateServerWithParams
	*/
	bool CreateServer(unsigned short Port);


	/**
	*	@brief	서버를 엽니다.
	*	@params	CreateServerParams - 서버를 열기위해 필요한 값들
	*	@return 성공 여부
	*	@see	struct CreateServerParams
	*/
	bool CreateServerWithParams(CreateServerParams& Param, GEventBinder& Event = EventBinderDefault);
	
	/**
	*	@brief	서버를 종료합니다.
	*	@details 모든 연결된 객체와 워커 스레드를 종료합니다.
	*/
	void Shutdown();

	void RegisterSocket(GSocket*);

	void TerminateSession(GSession&);

	/**
	*	@brief	워커쓰레드에서 호출되는 함수
	*	@params	IOCP Handle
	*	@return 스레드가 정상종료되면 0, 이외의 값은 에러와 함께 종료.
	*	@see	WorkerThread()
	*/
	unsigned int WorkerThreadProcess(void* pCompletionPort);

	unsigned int getHostIPAddress();
	HANDLE getCPHandle() { return CompletionPortHandle; }

	bool getbRunning() { return bRunning; }

private:
	std::map<SOCKET, GSocket*> SocketMap;
	GRWLock SocketMapLock; // Map에 Insert 하는 작업은 thread-safe하지 않으므로 관련 작업마다 Lock 처리를 해줌.

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