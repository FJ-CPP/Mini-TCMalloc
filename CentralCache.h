#pragma once
#include "PageHeap.h"

extern inline void*& NextObj(void*);

class CentralCache
{
private:
	// 单例模式(饿汉)
	static CentralCache _instance;
	CentralCache()
	{
	}
	CentralCache(const CentralCache&) = delete;
private:
	SpanList _spanLists[FREE_LIST_SIZE];
private:
	// 获取一个可用的Span
	Span* GetOneSpan(SpanList& list, size_t size);
public:
	static CentralCache* GetInstance()
	{
		return &_instance;
	}

	// 向Thread Cache提供n个size大小的内存块，返回实际提供了内存块数量
	int RemoveRange(void*& begin, void*& end, size_t n, size_t size);

	// 从Thread Cache回收size大小的内存块，并将其插入至原先的Span中
	void ReleaseListToSpans(void* begin, size_t size);
};

