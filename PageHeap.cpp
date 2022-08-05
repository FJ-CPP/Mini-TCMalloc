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

// ����һ��nҳ��Span
Span* PageHeap::NewSpan(size_t npage)
{
	assert(npage > 0);

	if (npage > MAX_PAGE_NUM - 1)
	{
		// ֱ����ϵͳ������
		Span* span = _spanPool.New();
		void* obj = SystemAlloc(npage);
		span->_pageID = (PAGE_ID)obj >> PAGE_SHIFT;
		span->_n = npage;
		span->_isUsed = true;
		_idMap.set(span->_pageID, span);
		return span;
	}

	// ����ʹ��ҳ��Ϊnpage��span
	if (!_spanLists[npage].Empty())
	{
		Span* span = _spanLists[npage].PopFront();
		span->_isUsed = true;
		// ��¼ id->Span ��ӳ���ϵ
		for (size_t i = 0; i < npage; ++i)
		{
			_idMap.set(span->_pageID + i, span);
		}
		return span;
	}

	// ���û��npage��Span����Ѱ��ӵ�и���page��Span�������з�
	for (size_t page = npage + 1; page < MAX_PAGE_NUM; ++page)
	{
		if (!_spanLists[page].Empty())
		{
			Span* oldSpan = _spanLists[page].PopFront();
			// ��span��Ϊҳ��Ϊ��npage �� page - npage ������Span
			Span* newSpan = _spanPool.New();

			newSpan->_n = npage;
			newSpan->_pageID = oldSpan->_pageID;
			newSpan->_isUsed = true;

			oldSpan->_n -= npage;
			oldSpan->_pageID += npage;

			// ���oldSpan��ǰ��߽磬����֮��ϲ�
			_idMap.set(oldSpan->_pageID, oldSpan);
			_idMap.set(oldSpan->_pageID + oldSpan->_n - 1, oldSpan);

			_spanLists[page - npage].PushFront(oldSpan); // ��ʣ���Span�����������

			// ��¼ id->Span ��ӳ���ϵ
			for (PAGE_ID i = 0; i < npage; ++i)
			{
				_idMap.set(newSpan->_pageID + i, newSpan);
			}

			return newSpan;
		}
	}

	// �ߵ�����˵��Ͱ�в����ڿ��õ�Span����ʱ��ϵͳ����һ�� MAX_PAGE_NUM - 1 ҳ��Span
	Span* span = _spanPool.New();
	void* ptr = SystemAlloc(MAX_PAGE_NUM - 1);
	span->_pageID = (PAGE_ID)ptr >> PAGE_SHIFT;
	span->_n = MAX_PAGE_NUM - 1;

	_spanLists[MAX_PAGE_NUM - 1].PushFront(span);

	return NewSpan(npage); // ���븴��
}

void PageHeap::ReleaseSpanToPageHeap(Span* span)
{
	// ����MAX_PAGE_NUM-1��ֱ�ӻ���ϵͳ��
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

	// ��ǰ�ϲ�
	while (true)
	{
		Span* ret = (Span*)_idMap.get(prevId);
		// û���ҵ���ӦSpan���߶�ӦSpan���ڱ�ʹ�ã���ֹͣ�ϲ�
		if (ret == nullptr || ret->_isUsed)
		{
			break;
		}
		else
		{
			Span* prevSpan = ret;

			// �ϲ��󳬹����ҳ������ֹͣ�ϲ�
			if (span->_n + prevSpan->_n > MAX_PAGE_NUM - 1)
			{
				break;
			}
			span->_pageID = prevSpan->_pageID;
			span->_n += prevSpan->_n;
			_spanLists[prevSpan->_n].Erase(prevSpan);
			_spanPool.Delete(prevSpan); // ���ϲ���preSpan��Ч�������ͷ�

			prevId = span->_pageID - 1;
		}
	}

	// ���ϲ�
	while (true)
	{
		Span* ret = (Span*)_idMap.get(nextId);
		// û���ҵ���ӦSpan���߶�ӦSpan���ڱ�ʹ�ã���ֹͣ�ϲ�
		if (ret == nullptr || ret->_isUsed)
		{
			break;
		}
		else
		{
			Span* nextSpan = ret;
			// �ϲ��󳬹����ҳ������ֹͣ�ϲ�
			if (span->_n + nextSpan->_n > MAX_PAGE_NUM - 1)
			{
				break;
			}
			span->_n += nextSpan->_n;
			_spanLists[nextSpan->_n].Erase(nextSpan);
			nextId = nextSpan->_pageID + nextSpan->_n;

			_spanPool.Delete(nextSpan); // ���ϲ���nextSpan��Ч�������ͷ�
		}
	}

	_spanLists[span->_n].PushFront(span);
	span->_isUsed = false;

	// ���Span��ǰ��߽磬���㱻����Span�ϲ�
	_idMap.set(span->_pageID, span);
	_idMap.set(span->_pageID + span->_n - 1, span);
}