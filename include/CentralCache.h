#pragma once
#include "PageHeap.h"

inline void *&next_obj(void *);

class CentralCache {
private:
  // 单例模式(饿汉)
  static CentralCache instance_;
  CentralCache() {}
  CentralCache(const CentralCache &) = delete;

private:
  SpanList span_lists_[FREE_LIST_SIZE];

private:
  // 获取一个可用的Span
  Span *get_one_span(SpanList &list, size_t size);

public:
  static CentralCache *get_instance() { return &instance_; }

  // 向Thread Cache提供n个size大小的内存块，返回实际提供了内存块数量
  int remove_range(void *&begin, void *&end, size_t n, size_t size);

  // 从Thread Cache回收size大小的内存块，并将其插入至原先的Span中
  void release_list_to_spans(void *begin, size_t size);
};
