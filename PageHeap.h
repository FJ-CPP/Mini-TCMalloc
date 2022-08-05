#pragma once
#include "SpanList.h"
#include "ObjectPool.hpp"
#include "PageMap.hpp"

class PageHeap
{
private:
	// ����ģʽ(����)
	static PageHeap _instance;
	PageHeap()
	{
	}
	PageHeap(const PageHeap&) = delete;
private:
	std::mutex _mtx;
	SpanList _spanLists[MAX_PAGE_NUM];
	TCMalloc_PageMap2<32 - PAGE_SHIFT> _idMap;
	ObjectPool<Span> _spanPool;
public:
	static PageHeap* GetInstance()
	{
		return &_instance;
	}

	// ��ȡ�ڴ�����ڵ�Span
	Span* MapObjectToSpan(void* obj);

	// ����һ��nҳ��Span
	Span* NewSpan(size_t npage);
	
	// ��Span������PageHeap
	void ReleaseSpanToPageHeap(Span* span);

	void Lock()
	{
		_mtx.lock();
	}

	void Unlock()
	{
		_mtx.unlock();
	}
};

