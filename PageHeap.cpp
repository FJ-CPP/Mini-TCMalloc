#define _CRT_SECURE_NO_WARNINGS 1
#include "PageHeap.h"

PageHeap PageHeap::_instance;

Span* PageHeap::MapObjectToSpan(void* obj)
{
	PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHIFT);

	Span* ret = (Span*)_idMap.get(id);

	if (ret != nullptr)
	{
		return ret;
	}
	else
	{
		assert(false);
		return nullptr;
	}
}

// 申请一个n页的Span
Span* PageHeap::NewSpan(size_t npage)
{
	assert(npage > 0);

	if (npage > MAX_PAGE_NUM - 1)
	{
		// 直接向系统堆申请
		Span* span = _spanPool.New();
		void* obj = SystemAlloc(npage);
		span->_pageID = (PAGE_ID)obj >> PAGE_SHIFT;
		span->_n = npage;
		span->_isUsed = true;
		_idMap.set(span->_pageID, span);
		return span;
	}

	// 优先使用页数为npage的span
	if (!_spanLists[npage].Empty())
	{
		Span* span = _spanLists[npage].PopFront();
		span->_isUsed = true;
		// 记录 id->Span 的映射关系
		for (size_t i = 0; i < npage; ++i)
		{
			_idMap.set(span->_pageID + i, span);
		}
		return span;
	}

	// 如果没有npage的Span，则寻找拥有更多page的Span并将其切分
	for (size_t page = npage + 1; page < MAX_PAGE_NUM; ++page)
	{
		if (!_spanLists[page].Empty())
		{
			Span* oldSpan = _spanLists[page].PopFront();
			// 将span分为页数为：npage 和 page - npage 的两个Span
			Span* newSpan = _spanPool.New();

			newSpan->_n = npage;
			newSpan->_pageID = oldSpan->_pageID;
			newSpan->_isUsed = true;

			oldSpan->_n -= npage;
			oldSpan->_pageID += npage;

			// 标记oldSpan的前后边界，方便之后合并
			_idMap.set(oldSpan->_pageID, oldSpan);
			_idMap.set(oldSpan->_pageID + oldSpan->_n - 1, oldSpan);

			_spanLists[page - npage].PushFront(oldSpan); // 将剩余的Span插入空闲链表

			// 记录 id->Span 的映射关系
			for (PAGE_ID i = 0; i < npage; ++i)
			{
				_idMap.set(newSpan->_pageID + i, newSpan);
			}

			return newSpan;
		}
	}

	// 走到这里说明桶中不存在可用的Span，此时向系统申请一个 MAX_PAGE_NUM - 1 页的Span
	Span* span = _spanPool.New();
	void* ptr = SystemAlloc(MAX_PAGE_NUM - 1);
	span->_pageID = (PAGE_ID)ptr >> PAGE_SHIFT;
	span->_n = MAX_PAGE_NUM - 1;

	_spanLists[MAX_PAGE_NUM - 1].PushFront(span);

	return NewSpan(npage); // 代码复用
}

void PageHeap::ReleaseSpanToPageHeap(Span* span)
{
	// 超过MAX_PAGE_NUM-1的直接还给系统堆
	if (span->_n > MAX_PAGE_NUM - 1)
	{
		void* obj = (void*)(span->_pageID >> PAGE_SHIFT);
		_spanPool.Delete(span);
		SystemFree(obj);
		return;
	}

	PAGE_ID id = span->_pageID;
	PAGE_ID prevId = id - 1;
	PAGE_ID nextId = id + span->_n;

	// 向前合并
	while (true)
	{
		Span* ret = (Span*)_idMap.get(prevId);
		// 没有找到对应Span或者对应Span正在被使用，则停止合并
		if (ret == nullptr || ret->_isUsed)
		{
			break;
		}
		else
		{
			Span* prevSpan = ret;

			// 合并后超过最大页数，则停止合并
			if (span->_n + prevSpan->_n > MAX_PAGE_NUM - 1)
			{
				break;
			}
			span->_pageID = prevSpan->_pageID;
			span->_n += prevSpan->_n;
			_spanLists[prevSpan->_n].Erase(prevSpan);
			_spanPool.Delete(prevSpan); // 被合并后preSpan无效，将其释放

			prevId = span->_pageID - 1;
		}
	}

	// 向后合并
	while (true)
	{
		Span* ret = (Span*)_idMap.get(nextId);
		// 没有找到对应Span或者对应Span正在被使用，则停止合并
		if (ret == nullptr || ret->_isUsed)
		{
			break;
		}
		else
		{
			Span* nextSpan = ret;
			// 合并后超过最大页数，则停止合并
			if (span->_n + nextSpan->_n > MAX_PAGE_NUM - 1)
			{
				break;
			}
			span->_n += nextSpan->_n;
			_spanLists[nextSpan->_n].Erase(nextSpan);
			nextId = nextSpan->_pageID + nextSpan->_n;

			_spanPool.Delete(nextSpan); // 被合并后nextSpan无效，将其释放
		}
	}

	_spanLists[span->_n].PushFront(span);
	span->_isUsed = false;

	// 标记Span的前后边界，方便被其他Span合并
	_idMap.set(span->_pageID, span);
	_idMap.set(span->_pageID + span->_n - 1, span);
}