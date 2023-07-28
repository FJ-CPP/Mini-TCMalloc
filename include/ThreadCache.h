#pragma once
#include "FreeList.h"
#include "Utility.h"
#include "CentralCache.h"

class ThreadCache {
private:
  FreeList free_lists_[FREE_LIST_SIZE];

private:
  // 从Central Cache获取内存块
  void *fetch_from_centralcache(size_t idx, size_t size);

public:
  // 分配内存
  void *allocate(size_t bytes);

  // 释放内存
  void deallocate(void *obj, size_t size);

  // 空闲链表过长，则将多余内存块还给Central Cache
  void list_too_long(FreeList &list, size_t size);

  // 释放之前将所有空闲链表还给Central Cache
  ~ThreadCache();
};

extern ObjectPool<ThreadCache> tc_pool;

class TLSThreadCache {
public:
  ThreadCache *ptc_;

  TLSThreadCache() {
    tc_pool.lock();
    ptc_ = tc_pool.New();
    tc_pool.unlock();
  }
  ~TLSThreadCache() {
    tc_pool.lock();
    tc_pool.Delete(ptc_);
    tc_pool.unlock();
  }
};
