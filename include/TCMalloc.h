#pragma once
#include "ThreadCache.h"

// Thread Cache数据结构池
static ObjectPool<ThreadCache> tc_pool;

class TLSThreadCache {
public:
  ThreadCache *ptc_;

public:
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

// 根据所需大小申请内存块
void *tcmalloc(int bytes);

// 释放TCMalloc申请的内存块obj
void tcfree(void *obj);