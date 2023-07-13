#pragma once
#include "common.h"

/*
 * 定长的数据结构内存池，根据模板T提供大小为sizeof(T)的定长内存块
 */

template <class T> class ObjectPool {
private:
  char *start_ = nullptr;     // 内存池的起始地址
  void *free_list_ = nullptr; // 可用内存块组成的空闲链表
  size_t remain_bytes_ = 0;   // 内存池剩余空间大小
  std::mutex mtx_;

public:
  // 申请一个T类型对象
  T *New() {
    return new T();
    T *obj = nullptr;
    // 优先到空闲链表中取下一个可用内存块
    if (free_list_ != nullptr) {
      obj = (T *)free_list_;
      free_list_ = *(void **)free_list_;

      new (obj) T;
      return obj;
    }

    // 直接从内存池取一块内存
    if (remain_bytes_ < sizeof(T)) {
      // 剩余内存不够用
      remain_bytes_ = 128 * 1024; // 每次默认开辟128KB
      start_ = (char *)system_alloc(remain_bytes_ >> 13);
    }

    // 由于freeList需要用"前4/8字节(取决于系统位数)"存储下一个节点
    // 因此至少分配sizeof(char*)个字节
    obj = (T *)start_;
    start_ += max(sizeof(T), sizeof(char *));
    remain_bytes_ -= max(sizeof(T), sizeof(char *));

    if (obj == nullptr) {
      int x = 10;
    }
    new (obj) T; // 定位new：显式调用构造函数
    return obj;
  }

  // 释放一个T类型对象
  void Delete(T *obj) {
    obj->~T(); // 清理对象

    *(void **)obj = free_list_;
    free_list_ = obj;
  }

  void lock() { mtx_.lock(); }

  void unlock() { mtx_.unlock(); }
};

// template <class T>
// class ObjectPool
//{
// private:
//	char* start_ = nullptr;  // 内存池的起始地址
//	FreeList free_list_;      // 可用内存块组成的空闲链表
//	size_t remain_bytes_ = 0; // 内存池剩余空间大小
// public:
//	// 申请一个T类型对象
//	T* New()
//	{
//		T* obj = nullptr;
//		// 优先到空闲链表中取下一个可用内存块
//		if (!free_list_.empty())
//		{
//			obj = (T*)free_list_.pop();
//
//			new(obj)T;
//			return obj;
//		}
//
//		// 直接从内存池取一块内存
//		if (remain_bytes_ < sizeof(T))
//		{
//			// 剩余内存不够用
//			remain_bytes_ = 128 * 1024; // 每次默认开辟128KB
//			start_ = (char*)system_alloc(remain_bytes_ >> 13);
//		}
//
//		// 由于freeList需要用"前4/8字节(取决于系统位数)"存储下一个节点
//		// 因此至少分配sizeof(char*)个字节
//		obj = (T*)start_;
//		start_ += max(sizeof(T), sizeof(char*));
//		remain_bytes_ -= max(sizeof(T), sizeof(char*));
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
//		free_list_.push(obj);
//	}
//};
