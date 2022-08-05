#pragma once
#include "common.h"

/*
 * ���������ݽṹ�ڴ�أ�����ģ��T�ṩ��СΪsizeof(T)�Ķ����ڴ��
*/

template <class T>
class ObjectPool
{
private:
	char* _start = nullptr;    // �ڴ�ص���ʼ��ַ
	void* _freeList = nullptr; // �����ڴ����ɵĿ�������
	size_t _remainBytes = 0;   // �ڴ��ʣ��ռ��С
	std::mutex _mtx;
public:
	// ����һ��T���Ͷ���
	T* New()
	{
		return new T();
		T* obj = nullptr;
		// ���ȵ�����������ȡ��һ�������ڴ��
		if (_freeList != nullptr)
		{
			obj = (T*)_freeList;
			_freeList = *(void**)_freeList;

			new(obj)T;
			return obj;
		}
		
		// ֱ�Ӵ��ڴ��ȡһ���ڴ�
		if (_remainBytes < sizeof(T))
		{
			// ʣ���ڴ治����
			_remainBytes = 128 * 1024; // ÿ��Ĭ�Ͽ���128KB
			_start = (char*)SystemAlloc(_remainBytes >> 13);
		}

		// ����freeList��Ҫ��"ǰ4/8�ֽ�(ȡ����ϵͳλ��)"�洢��һ���ڵ�
		// ������ٷ���sizeof(char*)���ֽ�
		obj = (T*)_start;
		_start += max(sizeof(T), sizeof(char*));
		_remainBytes -= max(sizeof(T), sizeof(char*));

		if (obj == nullptr)
		{
			int x = 10;
		}
		new(obj)T; // ��λnew����ʽ���ù��캯��
		return obj;
	}

	// �ͷ�һ��T���Ͷ���
	void Delete(T* obj)
	{
		obj->~T(); // �������

		*(void**)obj = _freeList;
		_freeList = obj;
	}

	void Lock()
	{
		_mtx.lock();
	}

	void Unlock()
	{
		_mtx.unlock();
	}
};

//template <class T>
//class ObjectPool
//{
//private:
//	char* _start = nullptr;  // �ڴ�ص���ʼ��ַ
//	FreeList _freeList;      // �����ڴ����ɵĿ�������
//	size_t _remainBytes = 0; // �ڴ��ʣ��ռ��С
//public:
//	// ����һ��T���Ͷ���
//	T* New()
//	{
//		T* obj = nullptr;
//		// ���ȵ�����������ȡ��һ�������ڴ��
//		if (!_freeList.Empty())
//		{
//			obj = (T*)_freeList.Pop();
//
//			new(obj)T;
//			return obj;
//		}
//
//		// ֱ�Ӵ��ڴ��ȡһ���ڴ�
//		if (_remainBytes < sizeof(T))
//		{
//			// ʣ���ڴ治����
//			_remainBytes = 128 * 1024; // ÿ��Ĭ�Ͽ���128KB
//			_start = (char*)SystemAlloc(_remainBytes >> 13);
//		}
//
//		// ����freeList��Ҫ��"ǰ4/8�ֽ�(ȡ����ϵͳλ��)"�洢��һ���ڵ�
//		// ������ٷ���sizeof(char*)���ֽ�
//		obj = (T*)_start;
//		_start += max(sizeof(T), sizeof(char*));
//		_remainBytes -= max(sizeof(T), sizeof(char*));
//
//		new(obj)T; // ��λnew����ʽ���ù��캯��
//		return obj;
//	}
//
//	// �ͷ�һ��T���Ͷ���
//	void Delete(T* obj)
//	{
//		obj->~T(); // �������
//
//		_freeList.Push(obj);
//	}
//};
