#include "Common.h"

#include <unordered_map>
static std::unordered_map<void *, int> kpage_map;

// 直接去堆上按页申请空间
void *system_alloc(size_t kpage) {
  void *ptr = nullptr;
#if _WIN32 || _WIN64
  ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE);
#elif __linux__
  ptr = mmap(0, kpage << PAGE_SHIFT, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) {
    ptr = nullptr;
  }
  kpage_map[ptr] = kpage;
#endif

  assert(ptr != nullptr);
  return ptr;
}

void system_free(void *ptr) {
#ifdef _WIN32
  VirtualFree(ptr, 0, MEM_RELEASE);
#elif __linux__
  int res = munmap(ptr, kpage_map[ptr] << PAGE_SHIFT);
  assert(res == 0);
  kpage_map.erase(ptr);
#endif
}
