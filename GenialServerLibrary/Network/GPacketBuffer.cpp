#include "GPacketBuffer.h"
#include "../Utility/GLogger.h"

GPacketBuffer::GPacketBuffer(uint64 BufferSize)
	: DataLength(0), isLittleEndian(CheckIsLittleEndian())
{
	AllocBuffer(BufferSize);
	isLittleEndian = CheckIsLittleEndian();
}

GPacketBuffer::GPacketBuffer()
	: DataLength(0), isLittleEndian(CheckIsLittleEndian())
{
	AllocBuffer(Default_BufferSize);
	isLittleEndian = CheckIsLittleEndian();
}

GPacketBuffer::~GPacketBuffer()
{
	free(Buffer);
}

void GPacketBuffer::Flush()
{
	CurOffsetPointer = Buffer;
	CurOffset = 0;
	DataLength = 0;
}

void GPacketBuffer::Flush(uint64 ParamDataLength)
{
	if (DataLength >= ParamDataLength)
		Flush();
	else
		memcpy(Buffer, CurOffsetPointer, DataLength - ParamDataLength);
}

void GPacketBuffer::Write(char* Data, int Length)
{
	// 데이터를 써야하는데 버퍼에 남은 공간이 부족하네?
	if (getRestSpace() > DataLength)
	{
		try
		{
			// Default_BufferSize 만큼 크기를 늘려줍시다.
			ExpandBufferSize(BufferSize += Default_BufferSize);
		}
		catch (GNetworkErrorCode ErrCode)
		{
			if (ErrCode == GNetworkErrorCode::MemoryFailure)
			{
				GLogger::Log("GPacketBuffer", "Write() failed - MemoryFailure\n");
				return;
			}
		}
	}
	memcpy(CurOffsetPointer, Data, Length);
	CurOffsetPointer += Length;
	CurOffset += Length;
	DataLength += Length;
}

void GPacketBuffer::CopyData(byte* Source, int Length)
{

	return;
}

