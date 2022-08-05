#pragma once
#include "common.h"

// ��������
class FreeList
{
private:
	void* _head = nullptr;
	size_t _size = 0;          // ��ǰ�����ڵ��ڴ���С
	size_t _length = 0;        // ��ǰ����ĳ���
	size_t _maxLength = 1;     // ��ǰ�������󳤶�(��������ֵ)
public:
	// ͷ��һ���ڵ�
	void Push(void* obj);

	// ȡ��ͷ���
	void* Pop(); 

	// ͷ��n�����
	void PushRange(void* begin, void* end, size_t n);

	// ͷɾn�����(����Ͳ���)
	void PopRange(void*& begin, void*& end, size_t n);
	
	// ��ȡͷ���
	void* Head() const
	{
		return _head;
	}

	// �ж��Ƿ�Ϊ��
	bool Empty() const
	{
		return _head == nullptr;
		//return _length == 0;
	}
	
	// ��ȡ�����ڴ洢���ڴ���С
	size_t Size() const
	{
		return _size;
	}

	// ���������ڴ洢���ڴ���С
	void SetSize(size_t size)
	{
		_size = size;
	}

	// ��ȡ��ǰ������
	size_t Length() const
	{
		return _length;
	}

	// ��ȡ��ǰ��������ֵ
	size_t MaxLength() const
	{
		return _maxLength;
	}

	// ���õ�ǰ��������ֵ
	void SetMaxLength(size_t newMax)
	{
		_maxLength = newMax;
	}
};