#include "Common.h"
#include "Static.h"

// 直接去堆上按页申请空间
void *system_alloc(size_t kpage) {
  void *ptr = nullptr;
#if _WIN32 || _WIN64
  ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE);
#elif __linux__
  ptr = mmap(0, (kpage << PAGE_SHIFT) + ALIGNMENT, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) {
    ptr = nullptr;
  } else {
    unsigned long aligned_addr =
        (((unsigned long)ptr + ALIGNMENT - 1) & ~(ALIGNMENT - 1));
    void *extra_start = ptr;
    size_t extra_size = aligned_addr - (unsigned long)ptr;
    if (extra_size > 0) {
      ASSERT(munmap(extra_start, extra_size) == 0);
    }

    void *extra_end = (void *)(aligned_addr + (kpage << PAGE_SHIFT));
    extra_size = (unsigned long)ptr + (kpage << PAGE_SHIFT) + ALIGNMENT -
                 (unsigned long)extra_end;
    if (extra_size > 0) {
      ASSERT(munmap(extra_end, extra_size) == 0);
    }

    ptr = reinterpret_cast<void *>(aligned_addr);
    ASSERT(aligned_addr % ALIGNMENT == 0);
    Static::kpage_map[ptr] = kpage;
  }
#endif

  ASSERT(ptr != nullptr);
  return ptr;
}

void system_free(void *ptr) {
#ifdef _WIN32
  VirtualFree(ptr, 0, MEM_RELEASE);
#elif __linux__
  ASSERT(munmap(ptr, Static::kpage_map[ptr] << PAGE_SHIFT) == 0);
  Static::kpage_map.erase(ptr);
#endif
}
