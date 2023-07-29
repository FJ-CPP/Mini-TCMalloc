#pragma once
#include <cassert>
#include <cstddef>

#include "Macro.hpp"

// 根据平台定义页大小
#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#elif __linux__ && __WORDSIZE == 64
typedef unsigned long long PAGE_ID;
#elif __linux__ && __WORDSIZE == 32
typedef size_t PAGE_ID;
#endif

// Thread Cache可分配的最大空间：256KB
const size_t MAX_BYTES = 256 * 1024;

// Thread Cache 和 Central Cache 管理空闲内存的哈希表大小
const size_t FREE_LIST_SIZE = 208;

// 一个span最多存储MAX_PAGE_NUM - 1页
const size_t MAX_PAGE_NUM = 129;

// 一个page占用2^PAGE_SHIFT字节的内存
#if defined _WIN64 || _WIN32
const int PAGE_SHIFT = 13;
#elif __linux__
const int PAGE_SHIFT = 12;
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/mman.h>
#endif

// 直接去堆上按页申请空间
void *system_alloc(size_t kpage);

// 直接去堆上按页释放空间
void system_free(void *ptr);
