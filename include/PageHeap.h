#pragma once
#include "Common.h"
#include "SpanList.h"
#include "ObjectPool.hpp"
#include "PageMap.hpp"

class PageHeap {
private:
  // 单例模式(饿汉)
  static PageHeap instance_;
  PageHeap() {}
  PageHeap(const PageHeap &) = delete;

private:
  std::mutex mtx_;
  SpanList span_lists_[MAX_PAGE_NUM];
  TCMalloc_PageMap2<32 - PAGE_SHIFT> id_map_;
  ObjectPool<Span> span_pool_;

public:
  static PageHeap *get_instance() { return &instance_; }

  // 获取内存块所在的Span
  Span *map_object_to_Span(void *obj);

  // 申请一个n页的Span
  Span *new_span(size_t npage);

  // 将Span返还给PageHeap
  void release_span_to_pageheap(Span *span);

  void lock() { mtx_.lock(); }

  void unlock() { mtx_.unlock(); }
};
