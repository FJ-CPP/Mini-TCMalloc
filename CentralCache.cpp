#define _CRT_SECURE_NO_WARNINGS 1

#include "Utility.h"
#include "CentralCache.h"

extern inline void*& NextObj(void*);

CentralCache CentralCache::_instance;

Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// ����ʹ�ÿ��е�span
	Span* span = list.Begin();
	while (span != list.End())
	{
		if (span->_freeList != nullptr)
		{
			return span;
		}
		span = span->_next;
	}

	// û�п��õ�span����Page Cache����

	list.Unlock(); // ��ʱ����Ҫ���ʹ�ϣͰ�ˣ�����Ȱ�Ͱ���⿪�����������̵߳�Thread Cache�����Ͱ�л��ڴ��

	PageHeap::GetInstance()->Lock();
	span = PageHeap::GetInstance()->NewSpan(Utility::NumMovePage(size));
	PageHeap::GetInstance()->Unlock();

	span->_objSize = size;

	char* begin = (char*)(span->_pageID << PAGE_SHIFT); // ���ڴ�����ʼλ��
	size_t bytes = span->_n << PAGE_SHIFT; // ���ڴ��Ĵ�С
	char* end = begin + bytes; // ���ڴ��Ľ���λ��

	// ��span����size�����ڴ���зֲ�β�嵽��������
	span->_freeList = begin;
	void* tail = begin;
	begin += size;
	while (begin < end)
	{
		NextObj(tail) = begin;
		tail = begin;
		begin += size;
	}
	NextObj(tail) = nullptr;

	// �з���ɺ����ŵ���Ӧ�Ĺ�ϣͰ��(���ڷ����˹�ϣͰ����ȼ���)
	list.Lock();
	list.PushFront(span);

	return span;
}

// ��Thread Cache�ṩn��size��С���ڴ�飬����ʵ���ṩ���ڴ������
int CentralCache::RemoveRange(void*& begin, void*& end, size_t n, size_t size)
{
	size_t idx = Utility::Index(size);
	SpanList& list = _spanLists[idx];

	list.Lock(); // ��Ͱ��

	Span* span = GetOneSpan(list, size); // ��ȡһ�����õ�Span
	assert(span);
	assert(span->_freeList);

	begin = span->_freeList;
	end = begin;
	size_t fetchNum = 1;

	// ȡ��fetchNum���ڴ�飬ֱ��fetchNum = n ���� ��������Ϊ��
	for (size_t i = 0; i < n - 1 && NextObj(end) != nullptr; ++i)
	{
		end = NextObj(end);
		++fetchNum;
	}
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;
	span->_useCount += fetchNum;

	list.Unlock();

	return fetchNum;
}

// ��Thread Cache����size��С���ڴ�飬�����������ԭ�ȵ�Span��
void CentralCache::ReleaseListToSpans(void* begin, size_t size)
{
	size_t idx = Utility::Index(size);
	SpanList& list = _spanLists[idx];

	list.Lock();

	while (begin)
	{
		void* next = NextObj(begin);

		// ���ڴ�黹����Ӧ��Span
		Span* span = PageHeap::GetInstance()->MapObjectToSpan(begin);

		NextObj(begin) = span->_freeList;
		span->_freeList = begin;
		span->_useCount--;

		if (span->_useCount == 0)
		{
			// span�ڷָ��ȥ���ڴ��ȫ����������
			// ��ʱ��span��ʼ��������Page Heap
			list.Erase(span);
			span->_freeList = nullptr;
			span->_prev = nullptr;
			span->_next = nullptr;

			// ��֮ǰ���Խ�Central Cache��Ͱ���⿪�����������߳�������ͷ��ڴ�
			list.Unlock();

			PageHeap::GetInstance()->Lock();
			PageHeap::GetInstance()->ReleaseSpanToPageHeap(span);
			PageHeap::GetInstance()->Unlock();

			// ��֮�������Ͱ��
			list.Lock();
		}

		begin = next;
	}

	list.Unlock();
}
