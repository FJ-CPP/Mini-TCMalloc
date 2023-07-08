#pragma once
#include "ThreadCache.h"

// Thread Cache数据结构池
static ObjectPool<ThreadCache> _tcPool;

class TLSThreadCache
{
public:
	ThreadCache* _pTC;
public:
	TLSThreadCache()
	{
		_tcPool.Lock();
		_pTC = _tcPool.New();
		_tcPool.Unlock();
	}
	~TLSThreadCache()
	{
		_tcPool.Lock();
		_tcPool.Delete(_pTC);
		_tcPool.Unlock();
	}
};



// 根据所需大小申请内存块
void* TCMalloc(int bytes);

// 释放TCMalloc申请的内存块obj
void TCFree(void* obj);