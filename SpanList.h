#pragma once
#include "common.h"
#include <mutex>

struct Span
{
	PAGE_ID _pageID = 0;       // Span����ʼҳҳ��
	size_t _objSize = 0;       // Span�г�ȥ��С�ڴ��Ĵ�С
	size_t _n = 0;             // ҳ�������
	int _useCount = 0;         // Span���ָ��С�ڴ����ж��ٸ��Ѿ���ʹ��

	void* _freeList = nullptr; // ����С�ڴ��Ŀ�������

	Span* _prev = nullptr;
	Span* _next = nullptr;

	bool _isUsed = false;
};

// ��ͷ˫���������Span
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