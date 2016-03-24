#pragma once

#include "GNetworkGlobal.h"
#include "../Utility/GRWLock.h"

/**
*	@brief		��Ŷ ���� Ŭ����. ������ ������ ���.
*	@details	��Ʋ ����� -> �򿣵�� ��ȯ�� �� Ŭ�������� ���
*	@todo		64��Ʈ ������ ���� �۾�&�׽�Ʈ �ʿ�
*	@todo		Memory Pooling ���
*	@author		������(whdcksz@gmail.com)
*/

class GPacketBuffer: public GRWLock
{
public:
	/**
	*	@brief ���� ����� ���Ƿ� ���ؼ� �Ҵ��ϴ� ������
	*/
	GPacketBuffer(uint64 BufferSize);

	/**
	*	@brief Default_BufferSize ��ŭ ���۸� �Ҵ��ϴ� ������
	*/
	GPacketBuffer();

	~GPacketBuffer();

	/**
	*	@brief ���� ��ü�� �����͸� ��������.
	*	@details �޸� ���簡 �Ͼ�� �ʾ� Flush(int)���� ����
	*/
	void Flush();

	/**
	*	@brief ������ �����͸� ���ϴ� ��ŭ ���������� ������ ���� ������ ��������Ʈ�� ����.
	*	@param �󸶸�ŭ Flush �� ������
	*	@see Flush()
	*/
	void Flush(uint64 DataLength);

	/**
	*	@brief ���ۿ� Data�� Length��ŭ �����۾�.
	*	@param Data�� ������
	*	@param Length, ����
	*	@todo ������ũ�Ⱑ �ʹ� Ŭ��� ��� Handle�� ������?
	*/
	void Write(char* Data, int Length);

	void CopyData(byte* Source, int Length);

	inline byte Read8(int Position);

	inline int16 Read16(int Position);

	inline int32 Read32(int Position);

	inline byte* const getBufferPointer();
	
	inline byte* const getCurrentOffsetPointer();

	inline void addDataLength(uint64 Size);

	inline void moveOffset(uint64 Size);

	inline uint64 getRestSpace();

	inline byte& operator[] (int Index);

	inline uint64 getDataLength();

private:
	bool isLittleEndian;	// �ü���� ����Ʈ������ ��Ʋ��������� ����
	byte* Buffer;			// ���� ���� ������
	uint64 BufferSize;			// ������ ������
	uint64 DataLength;		// ���� ���ۿ� ����Ǿ� �ִ� �������� ũ��
	uint64 CurOffset;
	byte* CurOffsetPointer;	// ���ۿ��� ���� �����Ͱ� ������ ��ġ�� ������

	void ExpandBufferSize(uint64 NewSize)
	{
		byte *OldBuffer = Buffer;
		AllocBuffer(NewSize);

		memcpy(Buffer, OldBuffer, DataLength);
		BufferSize = NewSize;
		free(OldBuffer);
	}

	void AllocBuffer(uint64 Size)
	{
		Buffer = (byte*)malloc(Size);
		if (Buffer == NULL)
			throw MemoryFailure;

		CurOffsetPointer = Buffer;
		CurOffset = 0;
		BufferSize = Size;
	}

	/**
	*	@brief ����Ʈ������ ��Ʋ��������� �˻��մϴ�.
	*	@details ����Ʈ������ ��Ʋ��������� Ȯ�εǸ� ���ۿ� ���� ������ �д� �������� ������ ��ȯ���ݴϴ�.
	*	@return ��Ʋ������� ��� true, �ƴҰ�� false
	*/
	inline bool CheckIsLittleEndian();

	/**
	*	@brief 16��Ʈ ������ ����Ʈ������ ��ȯ�մϴ�.
	*	@return ������� ��ȯ�� ��
	*/
	inline int16 Swap16(int16 Val);

	/**
	*	@brief 32��Ʈ ������ ����Ʈ������ ��ȯ�մϴ�.
	*	@return ������� ��ȯ�� ��
	*/
	inline int32 Swap32(int32 Val);
	
};

inline byte GPacketBuffer::Read8(int Position)
{
	return CurOffsetPointer[Position];
}

inline int16 GPacketBuffer::Read16(int Position)
{
	return *(int16*)(CurOffsetPointer + Position);
}

inline int32 GPacketBuffer::Read32(int Position)
{
	return *(int32*)(CurOffsetPointer + Position);
}

inline byte* const GPacketBuffer::getBufferPointer()
{
	return Buffer;
}

inline byte* const GPacketBuffer::getCurrentOffsetPointer()
{
	return CurOffsetPointer;
}

inline void GPacketBuffer::addDataLength(uint64 Size)
{
	DataLength += Size;
}

inline void GPacketBuffer::moveOffset(uint64 Size)
{
	CurOffset += Size;
	CurOffsetPointer += Size;
}

inline uint64 GPacketBuffer::getRestSpace()
{
	return BufferSize - DataLength;
}

inline uint64 GPacketBuffer::getDataLength()
{
	return DataLength;
}

inline byte& GPacketBuffer::operator[] (int Index)
{
	return Buffer[Index];
}

inline bool GPacketBuffer::CheckIsLittleEndian()
{
	int32 TestValue = 0x01234567;

	// �޸� �պκп� ���� �ڸ����� ���� �ִٸ�
	if ((*(byte*)&TestValue) == 0x67)
		return true; // ��Ʋ������Դϴ�.

	return false;
}

inline int16 GPacketBuffer::Swap16(int16 Val)
{
	if (isLittleEndian)
		return Val << 8 | Val >> 8;
	else
		return Val;
}

inline int32 GPacketBuffer::Swap32(int32 Val)
{
	if (isLittleEndian)
		return (Val << 24) | (Val << 8) & 0x00FF0000 | ((Val >> 8) & 0x0000FF00) | (Val >> 24);
	else
		return Val;
}