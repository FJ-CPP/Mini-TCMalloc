#pragma once
#include "common.h"

/*
 * 定长的数据结构内存池，根据模板T提供大小为sizeof(T)的定长内存块
 */

template <class T> class ObjectPool {
private:
  char *_start = nullptr;    // 内存池的起始地址
  void *_freeList = nullptr; // 可用内存块组成的空闲链表
  size_t _remainBytes = 0;   // 内存池剩余空间大小
  std::mutex _mtx;

public:
  // 申请一个T类型对象
  T *New() {
    return new T();
    T *obj = nullptr;
    // 优先到空闲链表中取下一个可用内存块
    if (_freeList != nullptr) {
      obj = (T *)_freeList;
      _freeList = *(void **)_freeList;

      new (obj) T;
      return obj;
    }

    // 直接从内存池取一块内存
    if (_remainBytes < sizeof(T)) {
      // 剩余内存不够用
      _remainBytes = 128 * 1024; // 每次默认开辟128KB
      _start = (char *)SystemAlloc(_remainBytes >> 13);
    }

    // 由于freeList需要用"前4/8字节(取决于系统位数)"存储下一个节点
    // 因此至少分配sizeof(char*)个字节
    obj = (T *)_start;
    _start += max(sizeof(T), sizeof(char *));
    _remainBytes -= max(sizeof(T), sizeof(char *));

    if (obj == nullptr) {
      int x = 10;
    }
    new (obj) T; // 定位new：显式调用构造函数
    return obj;
  }

  // 释放一个T类型对象
  void Delete(T *obj) {
    obj->~T(); // 清理对象

    *(void **)obj = _freeList;
    _freeList = obj;
  }

  void Lock() { _mtx.lock(); }

  void Unlock() { _mtx.unlock(); }
};

// template <class T>
// class ObjectPool
//{
// private:
//	char* _start = nullptr;  // 内存池的起始地址
//	FreeList _freeList;      // 可用内存块组成的空闲链表
//	size_t _remainBytes = 0; // 内存池剩余空间大小
// public:
//	// 申请一个T类型对象
//	T* New()
//	{
//		T* obj = nullptr;
//		// 优先到空闲链表中取下一个可用内存块
//		if (!_freeList.Empty())
//		{
//			obj = (T*)_freeList.Pop();
//
//			new(obj)T;
//			return obj;
//		}
//
//		// 直接从内存池取一块内存
//		if (_remainBytes < sizeof(T))
//		{
//			// 剩余内存不够用
//			_remainBytes = 128 * 1024; // 每次默认开辟128KB
//			_start = (char*)SystemAlloc(_remainBytes >> 13);
//		}
//
//		// 由于freeList需要用"前4/8字节(取决于系统位数)"存储下一个节点
//		// 因此至少分配sizeof(char*)个字节
//		obj = (T*)_start;
//		_start += max(sizeof(T), sizeof(char*));
//		_remainBytes -= max(sizeof(T), sizeof(char*));
//
//		new(obj)T; // 定位new：显式调用构造函数
//		return obj;
//	}
//
//	// 释放一个T类型对象
//	void Delete(T* obj)
//	{
//		obj->~T(); // 清理对象
//
//		_freeList.Push(obj);
//	}
//};
