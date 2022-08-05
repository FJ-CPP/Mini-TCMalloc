#define _CRT_SECURE_NO_WARNINGS 1
#include "ThreadCache.h"

void* ThreadCache::FetchFromCentralCache(size_t idx, size_t size)
{
	FreeList& list = _freeLists[idx];

	// ����������������
	size_t maxLength = list.MaxLength(); // ��������ֵ
	size_t batchNum = Utility::NumMoveSize(size); // �����ϵ�������������
	size_t numToFetch = min(maxLength, batchNum); // ʵ��Ӧȡ������

	// ������������������ڵ�ǰ��������ֵ������ֵ+1
	if (batchNum > maxLength)
	{
		list.SetMaxLength(maxLength + 1);
	}

	void* begin = nullptr;
	void* end = nullptr;

	// ��Central Cache����numToFetch���ڴ��
	int fetchCount = CentralCache::GetInstance()->RemoveRange(begin, end, numToFetch, size);
	assert(fetchCount > 0);

	if (fetchCount == 1)
	{
		assert(begin == end);
	}
	else
	{
		// ��ͷ�������Ķ����ڴ��Ĳ����������
		list.SetSize(size);
		list.PushRange(NextObj(begin), end, fetchCount - 1);
	}
	return begin;
}

void* ThreadCache::Allocate(size_t bytes)
{
	if (bytes > MAX_BYTES)
	{
		// ֱ����Central Cache����
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
		// ��Central Cache�����ڴ��
		return FetchFromCentralCache(idx, size);
	}
}

// �ͷ��ڴ�
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
	// ���ڴ��ȡ��maxLength��
	list.PopRange(begin, end, list.MaxLength());

	// ����Щ�ڴ�鷵����Central Cache
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