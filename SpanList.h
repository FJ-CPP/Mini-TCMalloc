#pragma once
#include "common.h"
#include <mutex>

struct Span
{
	PAGE_ID _pageID = 0;       // Span的起始页页号
	size_t _objSize = 0;       // Span切出去的小内存块的大小
	size_t _n = 0;             // 页面的数量
	int _useCount = 0;         // Span被分割成小内存块后，有多少个已经被使用

	void* _freeList = nullptr; // 管理小内存块的空闲链表

	Span* _prev = nullptr;
	Span* _next = nullptr;

	bool _isUsed = false;
};

// 带头双向链表管理Span
class SpanList
{
private:
	Span* _head;
	std::mutex _mtx;
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	bool Empty()
	{
		return _head->_next == _head;
	}

	Span* Begin()
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;
	}

	void PushFront(Span* newSpan);

	Span* PopFront();

	void Insert(Span* pos, Span* newSpan);

	void Erase(Span* pos);

	void Lock()
	{
		_mtx.lock();
	}

	void Unlock()
	{
		_mtx.unlock();
	}
};