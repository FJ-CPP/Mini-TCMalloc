#define _CRT_SECURE_NO_WARNINGS 1
#include "TCMalloc.h"

// TLS 线程本地存储的Thread Cache
thread_local TLSThreadCache tlsTC;

void *TCMalloc(int bytes) {
  assert(bytes > 0);

  // 超过Thread Cache能够分配的最大内存，因此直接向Page Heap申请
  if (bytes > MAX_BYTES) {
    int alignedSize = Utility::RoundUp(bytes);
    size_t npage = alignedSize >> PAGE_SHIFT;

    PageHeap::GetInstance()->Lock();
    Span *span = PageHeap::GetInstance()->NewSpan(npage);
    PageHeap::GetInstance()->Unlock();

    span->_objSize = alignedSize;
    void *obj = (void *)(span->_pageID << PAGE_SHIFT);
    return obj;
  }

  return tlsTC._pTC->Allocate(bytes);
}

void TCFree(void *obj) {
  Span *span = PageHeap::GetInstance()->MapObjectToSpan(obj);
  size_t size = span->_objSize;
  if (size > MAX_BYTES) {
    PageHeap::GetInstance()->Lock();
    PageHeap::GetInstance()->ReleaseSpanToPageHeap(span);
    PageHeap::GetInstance()->Unlock();
  } else {
    tlsTC._pTC->DeAllocate(obj, size);
  }
}
