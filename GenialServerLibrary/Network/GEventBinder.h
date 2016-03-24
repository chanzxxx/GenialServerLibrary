#pragma once

#include "GSession.h"

class GEventBinder
{
public:

	GEventBinder()
	{
	}

	virtual ~GEventBinder()
	{
	}

	enum onRecvReturnValue
	{
		Successful, // 성공적입니다. 다음 패킷을 대기합니다.
		Successful_Close, // 이 값을 리턴하면 세션을 종료합니다.
		Error_Close, // 이 값을 리턴하면 세션을 종료합니다.
		ProcessOnNextRecv // 이 패킷을 다음 Recv I/o 작업이 완료되었을때 처리도록 미뤄둡니다.
	};

	enum onAcceptReturnValue
	{
		Grant,
		Deny
	};

	/**
	*	@brief 패킷을 수신했을때 호출되는 이벤트입니다.
	*	@details 이 함수 내에서 패킷 Parse, 처리를 해주면 됩니다.
	*	@params 패킷을 수신한 세션의 레퍼런스
	*	@return Recv처리가 어떻게 되었는지 리턴해줘야 합니다.
	*/
	virtual onRecvReturnValue onRecv(GSession& Session)
	{
		return Successful;
	}

	/**
	*	@brief Connector가 Connect되었을때 호출되는 이벤트입니다.
	*	@params 서버에 접속한 세션의 레퍼런스
	*/
	virtual void onConnect(GSession& Session)
	{
		return;
	}

	/**
	*	@brief 클라이언트가 접속했을때 발생하는 이벤트입니다.
	*	@params 세션의 레퍼런스
	*	@details 이 함수 내에서 클라이언트 데이터를 초기화 하는 작업을 해주면 됩니다.
	*/
	virtual onAcceptReturnValue onAccept(GSession& Session)
	{
		
		return Grant;
	}

	/**
	*	@brief 클라이언트가 종료되었을 때 발생하는 이벤트입니다.
	*	@params 세션의 레퍼런스
	*	@details 이 함수 내에서 클라이언트 데이터를 삭제하는 작업을 해주면 됩니다.
	*/
	virtual void onClose(GSession& Session)
	{
		return;
	}
};

static GEventBinder EventBinderDefault;