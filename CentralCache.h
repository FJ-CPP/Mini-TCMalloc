#pragma once
#include "PageHeap.h"

extern inline void*& NextObj(void*);

class CentralCache
{
private:
	// ����ģʽ(����)
	static CentralCache _instance;
	CentralCache()
	{
	}
	CentralCache(const CentralCache&) = delete;
private:
	SpanList _spanLists[FREE_LIST_SIZE];
private:
	// ��ȡһ�����õ�Span
	Span* GetOneSpan(SpanList& list, size_t size);
public:
	static CentralCache* GetInstance()
	{
		return &_instance;
	}

	// ��Thread Cache�ṩn��size��С���ڴ�飬����ʵ���ṩ���ڴ������
	int RemoveRange(void*& begin, void*& end, size_t n, size_t size);

	// ��Thread Cache����size��С���ڴ�飬�����������ԭ�ȵ�Span��
	void ReleaseListToSpans(void* begin, size_t size);
};

