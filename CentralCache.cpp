#define _CRT_SECURE_NO_WARNINGS 1

#include "Utility.h"
#include "CentralCache.h"

extern inline void*& NextObj(void*);

CentralCache CentralCache::_instance;

Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// 优先使用空闲的span
	Span* span = list.Begin();
	while (span != list.End())
	{
		if (span->_freeList != nullptr)
		{
			return span;
		}
		span = span->_next;
	}

	// 没有可用的span则向Page Cache申请

	list.Unlock(); // 暂时不需要访问哈希桶了，因此先把桶锁解开，方便其它线程的Thread Cache向这个桶中还内存块

	PageHeap::GetInstance()->Lock();
	span = PageHeap::GetInstance()->NewSpan(Utility::NumMovePage(size));
	PageHeap::GetInstance()->Unlock();

	span->_objSize = size;

	char* begin = (char*)(span->_pageID << PAGE_SHIFT); // 大内存块的起始位置
	size_t bytes = span->_n << PAGE_SHIFT; // 大内存块的大小
	char* end = begin + bytes; // 大内存块的结束位置

	// 将span按照size进行内存块切分并尾插到空闲链表
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

	// 切分完成后将其存放到对应的哈希桶中(由于访问了哈希桶因此先加锁)
	list.Lock();
	list.PushFront(span);

	return span;
}

// 向Thread Cache提供n个size大小的内存块，返回实际提供了内存块数量
int CentralCache::RemoveRange(void*& begin, void*& end, size_t n, size_t size)
{
	size_t idx = Utility::Index(size);
	SpanList& list = _spanLists[idx];

	list.Lock(); // 加桶锁

	Span* span = GetOneSpan(list, size); // 获取一个可用的Span
	assert(span);
	assert(span->_freeList);

	begin = span->_freeList;
	end = begin;
	size_t fetchNum = 1;

	// 取下fetchNum个内存块，直至fetchNum = n 或者 空闲链表为空
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

// 从Thread Cache回收size大小的内存块，并将其插入至原先的Span中
void CentralCache::ReleaseListToSpans(void* begin, size_t size)
{
	size_t idx = Utility::Index(size);
	SpanList& list = _spanLists[idx];

	list.Lock();

	while (begin)
	{
		void* next = NextObj(begin);

		// 将内存块还给对应的Span
		Span* span = PageHeap::GetInstance()->MapObjectToSpan(begin);

		NextObj(begin) = span->_freeList;
		span->_freeList = begin;
		span->_useCount--;

		if (span->_useCount == 0)
		{
			// span内分割出去的内存块全部还回来了
			// 此时将span初始化并还给Page Heap
			list.Erase(span);
			span->_freeList = nullptr;
			span->_prev = nullptr;
			span->_next = nullptr;

			// 还之前可以将Central Cache的桶锁解开，方便其它线程申请和释放内存
			list.Unlock();

			PageHeap::GetInstance()->Lock();
			PageHeap::GetInstance()->ReleaseSpanToPageHeap(span);
			PageHeap::GetInstance()->Unlock();

			// 还之后继续加桶锁
			list.Lock();
		}

		begin = next;
	}

	list.Unlock();
}
