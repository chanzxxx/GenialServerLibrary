#include "GObjectPool.h"

GObjectPool* GObjectPool::Instance = NULL;

GObjectPool::GObjectPool()
{

}


GObjectPool::~GObjectPool()
{

}

void GObjectPool::Prepare(int ObjectCount, size_t size)
{
	unsigned int PoolID = (size / DefaultBlockSize) + 1;
	unsigned int MemorySize = PoolID*DefaultBlockSize + MemoryPadding;

	void *Chunk = malloc((MemorySize)*ObjectCount);
	//printf("prepare");
	if (Chunk == NULL)
	{
		printf("Malloc Fail\n");
	}

	ChunkList.push_back(Chunk);

	std::stack<void*> &FreeStackRef = FreeStack[PoolID];

	char *ChunkPointer = (char*)Chunk;
	for (unsigned int i = 0; i < ObjectCount; i++)
	{
		FreeStackRef.push(Chunk);
		ChunkPointer += MemorySize;
	}
}

void* GObjectPool::AllocObject(size_t size)
{
	unsigned int PoolID = (size / DefaultBlockSize) + 1;
	auto &FreeMemoryStack = FreeStack[PoolID];

	if (FreeMemoryStack.empty())
		Prepare(DefaultObjectCount, size);

	void *tmp = FreeMemoryStack.top();
	FreeMemoryStack.pop();

	return tmp;
}

void GObjectPool::FreeObject(void* Pointer, size_t size)
{
	unsigned int PoolID = (size / DefaultBlockSize) + 1;
	FreeStack[PoolID].push(Pointer);
}


/*
* ObjectPool Test Code
*/
/*
class A : public GObject
{
public:
	A() {}
	char Var[256];
};

class B
{
public:
	B() {}
	char Var[256];
};

int main()
{
	A *a[100000];
	B *b[100000];
	//GObjectPool::getInstance()->Prepare<A>(100000);

	DWORD start = timeGetTime();
	for (int i = 0; i < 100000; i++)
	{
		a[i] = new A;
	}

	for (int i = 0; i < 100000; i++)
	{
		delete a[i];
	}

	DWORD end = timeGetTime();
	printf("object pool perf: %d\n", end - start);
	start = timeGetTime();
	for (int i = 0; i < 100000; i++)
	{
		b[i] = new B;
	}

	for (int i = 0; i < 100000; i++)
	{
		delete b[i];
	}
	end = timeGetTime();
	printf("default new perf: %d\n", end - start);
	_getch();
}*/