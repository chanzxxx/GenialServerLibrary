#pragma once
#include "GRWLock.h"
#include <vector>
#include <list>
#include <memory.h>
#include <stdio.h>
#include <stack>

#define DefaultObjectCount 1024
#define DefaultBlockSize 32
#define MaxObjectSize 65536
#define MemoryPadding 8





/**
*	@brief		������Ʈ�� �޸��Ҵ�-���� ������带 ���̱����� ������Ʈ Ǯ Ŭ����
*	@todo		�����ؾ���
*/

class GObjectPool
{
public:
	GObjectPool();
	~GObjectPool();

	template <typename T>
	void Prepare(int ObjectCount);

	void Prepare(int ObjectCount, size_t size);

	template <typename T>
	T* AllocObject();

	void* AllocObject(size_t size);

	template <typename T>
	void FreeObject(T *Pointer);

	void FreeObject(void* Pointer, size_t size);

	inline static GObjectPool *getInstance()
	{
		if (Instance == NULL)
			Instance = new GObjectPool;

		return Instance;
	}

private:
	GRWLock Lock;
	void *Pool;
	int PoolSize;
	std::stack<void*> FreeStack[MaxObjectSize/DefaultBlockSize];
	std::list<void*> ChunkList; // ���߿� free ���ֱ� ���� ûũ �޸� �ּҸ� �� �����س���
	static GObjectPool *Instance;
};




template <typename T>
void GObjectPool::Prepare(int ObjectCount)
{
	Prepare(ObjectCount, sizeof(T));
}



template <typename T>
T* GObjectPool::AllocObject()
{
	return static_cast<T*>(AllocObject(sizeof(T)));
}



template <typename T>
void GObjectPool::FreeObject(T *Pointer)
{
	unsigned int PoolID = (sizeof(T) / DefaultBlockSize) + 1;
	FreeStack[PoolID].push(Pointer);
}


class GObject
{
public:
	void* operator new(size_t size)
	{
		return GObjectPool::getInstance()->AllocObject(size);
	}

	void operator delete(void* pointer, size_t size)
	{
		GObjectPool::getInstance()->FreeObject(pointer, size);
	}
};