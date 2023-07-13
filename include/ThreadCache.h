#pragma once
#include "FreeList.h"
#include "Utility.h"
#include "CentralCache.h"

class ThreadCache {
private:
  FreeList _freeLists[FREE_LIST_SIZE];

private:
  // 从Central Cache获取内存块
  void *FetchFromCentralCache(size_t idx, size_t size);

public:
  // 分配内存
  void *Allocate(size_t bytes);

  // 释放内存
  void DeAllocate(void *obj, size_t size);

  // 空闲链表过长，则将多余内存块还给Central Cache
  void ListTooLong(FreeList &list, size_t size);

  // 释放之前将所有空闲链表还给Central Cache
  ~ThreadCache();
};
