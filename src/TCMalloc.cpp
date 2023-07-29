#include "TCMalloc.h"
#include "ThreadCache.h"

// TLS 线程本地存储的Thread Cache
thread_local TLSThreadCache tls_tc;

void *tcmalloc(size_t bytes) {
  if (bytes == 0) {
    return nullptr;
  }
  ASSERT(bytes > 0);

  // 超过Thread Cache能够分配的最大内存，因此直接向Page Heap申请
  if (bytes > MAX_BYTES) {
    int aligned_size = Utility::round_up(bytes);
    size_t npage = aligned_size >> PAGE_SHIFT;

    PageHeap::get_instance()->lock();
    Span *span = PageHeap::get_instance()->new_span(npage);
    PageHeap::get_instance()->unlock();

    span->obj_size = aligned_size;
    void *obj = (void *)(span->page_id << PAGE_SHIFT);
    return obj;
  }

  return tls_tc.ptc_->allocate(bytes);
}

void tcfree(void *obj) {
  if (obj == nullptr) {
    ASSERT(false);
  }

  Span *span = PageHeap::get_instance()->map_object_to_Span(obj);
  size_t size = span->obj_size;
  if (size > MAX_BYTES) {
    PageHeap::get_instance()->lock();
    PageHeap::get_instance()->release_span_to_pageheap(span);
    PageHeap::get_instance()->unlock();
  } else {
    tls_tc.ptc_->deallocate(obj, size);
  }
}
