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
		Successful, // �������Դϴ�. ���� ��Ŷ�� ����մϴ�.
		Successful_Close, // �� ���� �����ϸ� ������ �����մϴ�.
		Error_Close, // �� ���� �����ϸ� ������ �����մϴ�.
		ProcessOnNextRecv // �� ��Ŷ�� ���� Recv I/o �۾��� �Ϸ�Ǿ����� ó������ �̷�Ӵϴ�.
	};

	enum onAcceptReturnValue
	{
		Grant,
		Deny
	};

	/**
	*	@brief ��Ŷ�� ���������� ȣ��Ǵ� �̺�Ʈ�Դϴ�.
	*	@details �� �Լ� ������ ��Ŷ Parse, ó���� ���ָ� �˴ϴ�.
	*	@params ��Ŷ�� ������ ������ ���۷���
	*	@return Recvó���� ��� �Ǿ����� ��������� �մϴ�.
	*/
	virtual onRecvReturnValue onRecv(GSession& Session)
	{
		return Successful;
	}

	/**
	*	@brief Connector�� Connect�Ǿ����� ȣ��Ǵ� �̺�Ʈ�Դϴ�.
	*	@params ������ ������ ������ ���۷���
	*/
	virtual void onConnect(GSession& Session)
	{
		return;
	}

	/**
	*	@brief Ŭ���̾�Ʈ�� ���������� �߻��ϴ� �̺�Ʈ�Դϴ�.
	*	@params ������ ���۷���
	*	@details �� �Լ� ������ Ŭ���̾�Ʈ �����͸� �ʱ�ȭ �ϴ� �۾��� ���ָ� �˴ϴ�.
	*/
	virtual onAcceptReturnValue onAccept(GSession& Session)
	{
		
		return Grant;
	}

	/**
	*	@brief Ŭ���̾�Ʈ�� ����Ǿ��� �� �߻��ϴ� �̺�Ʈ�Դϴ�.
	*	@params ������ ���۷���
	*	@details �� �Լ� ������ Ŭ���̾�Ʈ �����͸� �����ϴ� �۾��� ���ָ� �˴ϴ�.
	*/
	virtual void onClose(GSession& Session)
	{
		return;
	}
};

static GEventBinder EventBinderDefault;