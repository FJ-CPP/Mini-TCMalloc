#pragma once
#include "SpanList.h"
#include "ObjectPool.hpp"
#include "PageMap.hpp"

class PageHeap {
private:
  // 单例模式(饿汉)
  static PageHeap _instance;
  PageHeap() {}
  PageHeap(const PageHeap &) = delete;

private:
  std::mutex _mtx;
  SpanList _spanLists[MAX_PAGE_NUM];
  TCMalloc_PageMap2<32 - PAGE_SHIFT> _idMap;
  ObjectPool<Span> _spanPool;

public:
  static PageHeap *GetInstance() { return &_instance; }

  // 获取内存块所在的Span
  Span *MapObjectToSpan(void *obj);

  // 申请一个n页的Span
  Span *NewSpan(size_t npage);

  // 将Span返还给PageHeap
  void ReleaseSpanToPageHeap(Span *span);

  void Lock() { _mtx.lock(); }

  void Unlock() { _mtx.unlock(); }
};
