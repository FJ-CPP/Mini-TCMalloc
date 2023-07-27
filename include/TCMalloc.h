#pragma once

#include <cstddef>

#ifdef _WIN32
#define MINI_TCMALLOC_EXPORT_SYMBOL __declspec(dllexport)
#define MINI_TCMALLOC_IMPORT_SYMBOL __declspec(dllimport)
#else
#define MINI_TCMALLOC_EXPORT_SYMBOL __attribute__((visibility("default")))
#define MINI_TCMALLOC_IMPORT_SYMBOL
#endif

// 根据所需大小申请内存块
MINI_TCMALLOC_EXPORT_SYMBOL void *tcmalloc(size_t bytes);

// 释放TCMalloc申请的内存块obj
MINI_TCMALLOC_EXPORT_SYMBOL void tcfree(void *obj);