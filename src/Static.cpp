#include "Static.h"
#include "CentralCache.h"
#include "ThreadCache.h"

std::unordered_map<void *, int> Static::kpage_map;

ObjectPool<Span> Static::static_span_pool;

ObjectPool<ThreadCache> tc_pool;

CentralCache CentralCache::instance_;

PageHeap PageHeap::instance_;
