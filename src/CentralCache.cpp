#include "Utility.h"
#include "CentralCache.h"

Span *CentralCache::get_one_span(SpanList &list, size_t size) {
  // 优先使用空闲的span
  Span *span = list.begin();
  while (span != list.end()) {
    if (span->free_list != nullptr) {
      return span;
    }
    span = span->next;
  }

  // 没有可用的span则向Page Cache申请

  list.unlock(); // 暂时不需要访问哈希桶了，因此先把桶锁解开，方便其它线程的Thread
                 // Cache向这个桶中还内存块

  PageHeap::get_instance()->lock();
  span = PageHeap::get_instance()->new_span(Utility::num_move_page(size));
  PageHeap::get_instance()->unlock();

  span->obj_size = size;

  char *begin = (char *)(span->page_id << PAGE_SHIFT); // 大内存块的起始位置
  size_t bytes = span->n << PAGE_SHIFT;                // 大内存块的大小
  char *end = begin + bytes; // 大内存块的结束位置

  // 将span按照size进行内存块切分并尾插到空闲链表
  span->free_list = begin;
  void *tail = begin;
  begin += size;
  while (begin < end) {
    next_obj(tail) = begin;
    tail = begin;
    begin += size;
  }
  next_obj(tail) = nullptr;

  // 切分完成后将其存放到对应的哈希桶中(由于访问了哈希桶因此先加锁)
  list.lock();
  list.push_front(span);

  return span;
}

// 向Thread Cache提供n个size大小的内存块，返回实际提供了内存块数量
int CentralCache::remove_range(void *&begin, void *&end, size_t n,
                               size_t size) {
  size_t idx = Utility::index(size);
  SpanList &list = span_lists_[idx];

  list.lock(); // 加桶锁

  Span *span = get_one_span(list, size); // 获取一个可用的Span
  ASSERT(span != nullptr);
  ASSERT(span->free_list);

  begin = span->free_list;
  end = begin;
  size_t fetchNum = 1;

  // 取下fetchNum个内存块，直至fetchNum = n 或者 空闲链表为空
  for (size_t i = 0; i < n - 1 && next_obj(end) != nullptr; ++i) {
    end = next_obj(end);
    ++fetchNum;
  }
  span->free_list = next_obj(end);
  next_obj(end) = nullptr;
  span->use_count += fetchNum;

  list.unlock();

  return fetchNum;
}

// 从Thread Cache回收size大小的内存块，并将其插入至原先的Span中
void CentralCache::release_list_to_spans(void *begin, size_t size) {
  size_t idx = Utility::index(size);
  SpanList &list = span_lists_[idx];

  list.lock();

  while (begin) {
    void *next = next_obj(begin);

    // 将内存块还给对应的Span
    Span *span = PageHeap::get_instance()->map_object_to_Span(begin);

    next_obj(begin) = span->free_list;
    span->free_list = begin;
    span->use_count--;

    if (span->use_count == 0) {
      // span内分割出去的内存块全部还回来了
      // 此时将span初始化并还给Page Heap
      list.erase(span);
      span->free_list = nullptr;
      span->prev = nullptr;
      span->next = nullptr;

      // 还之前可以将Central Cache的桶锁解开，方便其它线程申请和释放内存
      list.unlock();

      PageHeap::get_instance()->lock();
      PageHeap::get_instance()->release_span_to_pageheap(span);
      PageHeap::get_instance()->unlock();

      // 还之后继续加桶锁
      list.lock();
    }

    begin = next;
  }

  list.unlock();
}
