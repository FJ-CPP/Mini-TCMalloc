#pragma once
#include <cassert>

// ����ƽ̨����ҳ��С
#ifdef _WIN64 
	typedef unsigned long long PAGE_ID;
#elif _WIN32
	typedef size_t PAGE_ID;
#elif __linux__ && __WORDSIZE == 64
	typedef unsigned long long PAGE_ID;
#elif __linux__ && __WORDSZIE == 32
	typedef unsigned int PAGE_ID;
#endif

// Thread Cache�ɷ�������ռ䣺256KB
const size_t MAX_BYTES = 256 * 1024;

// Thread Cache �� Central Cache ��������ڴ�Ĺ�ϣ���С
const size_t FREE_LIST_SIZE = 208;

// һ��span���洢MAX_PAGE_NUM - 1ҳ
const size_t MAX_PAGE_NUM = 129;

// һ��pageռ��2^PAGE_SHIFT�ֽڵ��ڴ�
const int PAGE_SHIFT = 13;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/mman.h>
#endif

// ֱ��ȥ���ϰ�ҳ����ռ�
inline static void* SystemAlloc(size_t kpage)
{
	void* ptr = nullptr;
#if _WIN32 || _WIN64
	ptr = VirtualAlloc(0, kpage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif __linux__
	// Linux��ʹ��sbrk��mmap�����ڴ�(������Ҫ���������Զ���)
#endif

	assert(ptr != nullptr);
	return ptr;
}

inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#elif __linux__
	// Linux��ʹ��sbrk��unmmap�ͷ��ڴ�
#endif
}

