#pragma once
#include <cassert>
#include <cstddef>

// 根据平台定义页大小
#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#elif __linux__ && __WORDSIZE == 64
typedef unsigned long long PAGE_ID;
#elif __linux__ && __WORDSIZE == 32
typedef unsigned int PAGE_ID;
#endif

// Thread Cache可分配的最大空间：256KB
const size_t MAX_BYTES = 256 * 1024;

// Thread Cache 和 Central Cache 管理空闲内存的哈希表大小
const size_t FREE_LIST_SIZE = 208;

// 一个span最多存储MAX_PAGE_NUM - 1页
const size_t MAX_PAGE_NUM = 129;

// 一个page占用2^PAGE_SHIFT字节的内存
const int PAGE_SHIFT = 12;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/mman.h>
#endif

#include <unordered_map>
static std::unordered_map<void *, int> kpage_map;

// 直接去堆上按页申请空间
inline static void *system_alloc(size_t kpage) {
  void *ptr = nullptr;
#if _WIN32 || _WIN64
  ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE);
#elif __linux__
  // Linux下使用sbrk和mmap申请内存(可能需要两次申请以对齐)
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

inline static void system_free(void *ptr) {
#ifdef _WIN32
  VirtualFree(ptr, 0, MEM_RELEASE);
#elif __linux__
  // Linux下使用sbrk和unmmap释放内存
  int res = munmap(ptr, kpage_map[ptr] << PAGE_SHIFT);
  assert(res == 0);
  kpage_map.erase(ptr);
#endif
}
