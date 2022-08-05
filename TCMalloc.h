#pragma once
#include "ThreadCache.h"

// Thread Cache���ݽṹ��
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



// ���������С�����ڴ��
void* TCMalloc(int bytes);

// �ͷ�TCMalloc������ڴ��obj
void TCFree(void* obj);