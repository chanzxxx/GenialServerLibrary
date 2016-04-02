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
*	@brief		오브젝트의 메모리할당-해제 오버헤드를 줄이기위한 오브젝트 풀 클래스
*	@todo		구현해야함
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
	std::list<void*> ChunkList; // 나중에 free 해주기 위해 청크 메모리 주소를 다 저장해놓기
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