#pragma once
#include <cassert>

// 根据平台定义页大小
#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#elif __linux__ && __WORDSIZE == 64
typedef unsigned long long PAGE_ID;
#elif __linux__ && __WORDSZIE == 32
typedef unsigned int PAGE_ID;
#endif

// Thread Cache可分配的最大空间：256KB
const size_t MAX_BYTES = 256 * 1024;

// Thread Cache 和 Central Cache 管理空闲内存的哈希表大小
const size_t FREE_LIST_SIZE = 208;

// 一个span最多存储MAX_PAGE_NUM - 1页
const size_t MAX_PAGE_NUM = 129;

// 一个page占用2^PAGE_SHIFT字节的内存
const int PAGE_SHIFT = 13;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/mman.h>
#endif

// 直接去堆上按页申请空间
inline static void *SystemAlloc(size_t kpage) {
  void *ptr = nullptr;
#if _WIN32 || _WIN64
  ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE);
#elif __linux__
  // Linux下使用sbrk和mmap申请内存(可能需要两次申请以对齐)
#endif

  assert(ptr != nullptr);
  return ptr;
}

inline static void SystemFree(void *ptr) {
#ifdef _WIN32
  VirtualFree(ptr, 0, MEM_RELEASE);
#elif __linux__
  // Linux下使用sbrk和unmmap释放内存
#endif
}
