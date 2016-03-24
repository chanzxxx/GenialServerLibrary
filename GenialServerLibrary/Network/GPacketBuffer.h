#pragma once

#include "GNetworkGlobal.h"
#include "../Utility/GRWLock.h"

/**
*	@brief		패킷 버퍼 클래스. 버퍼의 관리를 담당.
*	@details	리틀 엔디안 -> 빅엔디안 변환도 이 클래스에서 담당
*	@todo		64비트 지원을 위한 작업&테스트 필요
*	@todo		Memory Pooling 기능
*	@author		박종찬(whdcksz@gmail.com)
*/

class GPacketBuffer: public GRWLock
{
public:
	/**
	*	@brief 버퍼 사이즈를 임의로 정해서 할당하는 생성자
	*/
	GPacketBuffer(uint64 BufferSize);

	/**
	*	@brief Default_BufferSize 만큼 버퍼를 할당하는 생성자
	*/
	GPacketBuffer();

	~GPacketBuffer();

	/**
	*	@brief 버퍼 전체의 데이터를 날려버림.
	*	@details 메모리 복사가 일어나지 않아 Flush(int)보다 빠름
	*/
	void Flush();

	/**
	*	@brief 버퍼의 데이터를 원하는 만큼 날려버리고 나머지 값을 버퍼의 시작포인트로 복사.
	*	@param 얼마만큼 Flush 할 것인지
	*	@see Flush()
	*/
	void Flush(uint64 DataLength);

	/**
	*	@brief 버퍼에 Data를 Length만큼 쓰기작업.
	*	@param Data의 포인터
	*	@param Length, 길이
	*	@todo 데이터크기가 너무 클경우 어떻게 Handle할 것인지?
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
	bool isLittleEndian;	// 운영체제의 바이트오더가 리틀엔디안인지 저장
	byte* Buffer;			// 실제 버퍼 포인터
	uint64 BufferSize;			// 버퍼의 사이즈
	uint64 DataLength;		// 현재 버퍼에 저장되어 있는 데이터의 크기
	uint64 CurOffset;
	byte* CurOffsetPointer;	// 버퍼에서 다음 데이터가 쓰여질 위치의 포인터

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
	*	@brief 바이트오더가 리틀엔디언인지 검사합니다.
	*	@details 바이트오더가 리틀엔디언으로 확인되면 버퍼에 쓰는 과정과 읽는 과정에서 적절히 변환해줍니다.
	*	@return 리틀엔디언일 경우 true, 아닐경우 false
	*/
	inline bool CheckIsLittleEndian();

	/**
	*	@brief 16비트 변수의 바이트오더를 변환합니다.
	*	@return 엔디언간의 변환된 값
	*/
	inline int16 Swap16(int16 Val);

	/**
	*	@brief 32비트 변수의 바이트오더를 변환합니다.
	*	@return 엔디언간의 변환된 값
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

	// 메모리 앞부분에 작은 자리수의 값이 있다면
	if ((*(byte*)&TestValue) == 0x67)
		return true; // 리틀엔디안입니다.

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