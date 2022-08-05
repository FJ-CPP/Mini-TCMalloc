#pragma once
#include "FreeList.h"
#include "Utility.h"
#include "CentralCache.h"

class ThreadCache
{
private:
	FreeList _freeLists[FREE_LIST_SIZE];
private:
	// ��Central Cache��ȡ�ڴ��
	void* FetchFromCentralCache(size_t idx, size_t size);
public:
	// �����ڴ�
	void* Allocate(size_t bytes);

	// �ͷ��ڴ�
	void DeAllocate(void* obj, size_t size);

	// ��������������򽫶����ڴ�黹��Central Cache
	void ListTooLong(FreeList& list, size_t size);

	// �ͷ�֮ǰ�����п���������Central Cache
	~ThreadCache();
};
