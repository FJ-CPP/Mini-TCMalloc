#define _CRT_SECURE_NO_WARNINGS 1
#include "ThreadCache.h"

void* ThreadCache::FetchFromCentralCache(size_t idx, size_t size)
{
	FreeList& list = _freeLists[idx];

	// 批量申请与慢启动
	size_t maxLength = list.MaxLength(); // 慢启动阈值
	size_t batchNum = Utility::NumMoveSize(size); // 理论上的批量申请数量
	size_t numToFetch = min(maxLength, batchNum); // 实际应取的数量

	// 如果批量申请数量大于当前慢启动阈值，则阈值+1
	if (batchNum > maxLength)
	{
		list.SetMaxLength(maxLength + 1);
	}

	void* begin = nullptr;
	void* end = nullptr;

	// 向Central Cache申请numToFetch个内存块
	int fetchCount = CentralCache::GetInstance()->RemoveRange(begin, end, numToFetch, size);
	assert(fetchCount > 0);

	if (fetchCount == 1)
	{
		assert(begin == end);
	}
	else
	{
		// 将头结点以外的多余内存块的插入空闲链表
		list.SetSize(size);
		list.PushRange(NextObj(begin), end, fetchCount - 1);
	}
	return begin;
}

void* ThreadCache::Allocate(size_t bytes)
{
	if (bytes > MAX_BYTES)
	{
		// 直接向Central Cache申请
		// TODO
	}

	size_t size = Utility::RoundUp(bytes);
	int idx = Utility::Index(size);

	if (!_freeLists[idx].Empty())
	{
		return _freeLists[idx].Pop();
	}
	else
	{
		// 向Central Cache申请内存块
		return FetchFromCentralCache(idx, size);
	}
}

// 释放内存
void ThreadCache::DeAllocate(void* obj, size_t size)
{
	assert(obj != nullptr);
	int idx = Utility::Index(size);

	FreeList& list = _freeLists[idx];
	list.Push(obj);

	if (list.Length() >= list.MaxLength())
	{
		ListTooLong(list, size);
	}
}

void ThreadCache::ListTooLong(FreeList& list, size_t size)
{
	void* begin = nullptr;
	void* end = nullptr;
	// 将内存块取下maxLength个
	list.PopRange(begin, end, list.MaxLength());

	// 将这些内存块返还给Central Cache
	CentralCache::GetInstance()->ReleaseListToSpans(begin, size);
}

ThreadCache::~ThreadCache()
{
	for (auto& list : _freeLists)
	{
		if (!list.Empty())
		{
			CentralCache::GetInstance()->ReleaseListToSpans(list.Head(), list.Size());
		}
	}
}