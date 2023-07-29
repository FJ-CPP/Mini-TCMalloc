#include "PageHeap.h"

PageHeap PageHeap::instance_;

Span *PageHeap::map_object_to_Span(void *obj) {
  PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHIFT);

  Span *ret = (Span *)id_map_.get(id);

  if (ret != nullptr) {
    return ret;
  } else {
    ASSERT(false);
    return nullptr;
  }
}

// 申请一个n页的Span
Span *PageHeap::new_span(size_t npage) {
  ASSERT(npage > 0);

  if (npage > MAX_PAGE_NUM - 1) {
    // 直接向系统堆申请
    Span *span = span_pool_.New();
    void *obj = system_alloc(npage);
    span->page_id = (PAGE_ID)obj >> PAGE_SHIFT;
    span->n = npage;
    span->is_used = true;
    id_map_.set(span->page_id, span);
    return span;
  }

  // 优先使用页数为npage的span
  if (!span_lists_[npage].empty()) {
    Span *span = span_lists_[npage].pop_front();
    span->is_used = true;
    // 记录 id->Span 的映射关系
    for (size_t i = 0; i < npage; ++i) {
      id_map_.set(span->page_id + i, span);
    }
    return span;
  }

  // 如果没有npage的Span，则寻找拥有更多page的Span并将其切分
  for (size_t page = npage + 1; page < MAX_PAGE_NUM; ++page) {
    if (!span_lists_[page].empty()) {
      Span *oldSpan = span_lists_[page].pop_front();
      // 将span分为页数为：npage 和 page - npage 的两个Span
      Span *newSpan = span_pool_.New();

      newSpan->n = npage;
      newSpan->page_id = oldSpan->page_id;
      newSpan->is_used = true;

      oldSpan->n -= npage;
      oldSpan->page_id += npage;

      // 标记oldSpan的前后边界，方便之后合并
      id_map_.set(oldSpan->page_id, oldSpan);
      id_map_.set(oldSpan->page_id + oldSpan->n - 1, oldSpan);

      span_lists_[page - npage].push_front(oldSpan); // 将剩余的Span插入空闲链表

      // 记录 id->Span 的映射关系
      for (PAGE_ID i = 0; i < npage; ++i) {
        id_map_.set(newSpan->page_id + i, newSpan);
      }

      return newSpan;
    }
  }

  // 走到这里说明桶中不存在可用的Span，此时向系统申请一个 MAX_PAGE_NUM - 1
  // 页的Span
  Span *span = span_pool_.New();
  void *ptr = system_alloc(MAX_PAGE_NUM - 1);
  span->page_id = (PAGE_ID)ptr >> PAGE_SHIFT;
  span->n = MAX_PAGE_NUM - 1;

  span_lists_[MAX_PAGE_NUM - 1].push_front(span);

  return new_span(npage); // 代码复用
}

void PageHeap::release_span_to_pageheap(Span *span) {
  // 超过MAX_PAGE_NUM-1的直接还给系统堆
  if (span->n > MAX_PAGE_NUM - 1) {
    void *obj = (void *)(span->page_id << PAGE_SHIFT);
    span_pool_.Delete(span);
    system_free(obj);
    return;
  }

  PAGE_ID id = span->page_id;
  PAGE_ID prevId = id - 1;
  PAGE_ID nextId = id + span->n;

  // 向前合并
  while (true) {
    Span *ret = (Span *)id_map_.get(prevId);
    // 没有找到对应Span或者对应Span正在被使用，则停止合并
    if (ret == nullptr || ret->is_used) {
      break;
    } else {
      Span *prev_span = ret;

      // 合并后超过最大页数，则停止合并
      if (span->n + prev_span->n > MAX_PAGE_NUM - 1) {
        break;
      }
      span->page_id = prev_span->page_id;
      span->n += prev_span->n;
      span_lists_[prev_span->n].erase(prev_span);
      span_pool_.Delete(prev_span); // 被合并后preSpan无效，将其释放

      prevId = span->page_id - 1;
    }
  }

  // 向后合并
  while (true) {
    Span *ret = (Span *)id_map_.get(nextId);
    // 没有找到对应Span或者对应Span正在被使用，则停止合并
    if (ret == nullptr || ret->is_used) {
      break;
    } else {
      Span *next_span = ret;
      // 合并后超过最大页数，则停止合并
      if (span->n + next_span->n > MAX_PAGE_NUM - 1) {
        break;
      }
      span->n += next_span->n;
      span_lists_[next_span->n].erase(next_span);
      nextId = next_span->page_id + next_span->n;

      span_pool_.Delete(next_span); // 被合并后nextSpan无效，将其释放
    }
  }

  span_lists_[span->n].push_front(span);
  span->is_used = false;

  // 标记Span的前后边界，方便被其他Span合并
  id_map_.set(span->page_id, span);
  id_map_.set(span->page_id + span->n - 1, span);
}