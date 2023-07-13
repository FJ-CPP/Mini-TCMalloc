#include "ThreadCache.h"

void *ThreadCache::fetch_from_centralcache(size_t idx, size_t size) {
  FreeList &list = free_lists_[idx];

  // 批量申请与慢启动
  size_t max_length = list.max_length();           // 慢启动阈值
  size_t batch_num = Utility::num_move_size(size); // 理论上的批量申请数量
  size_t num_to_fetch = std::min(max_length, batch_num); // 实际应取的数量

  // 如果批量申请数量大于当前慢启动阈值，则阈值+1
  if (batch_num > max_length) {
    list.set_max_length(max_length + 1);
  }

  void *begin = nullptr;
  void *end = nullptr;

  // 向Central Cache申请numToFetch个内存块
  int fetch_count = CentralCache::get_instance()->remove_range(
      begin, end, num_to_fetch, size);
  assert(fetch_count > 0);

  if (fetch_count == 1) {
    assert(begin == end);
  } else {
    // 将头结点以外的多余内存块的插入空闲链表
    list.set_size(size);
    list.push_range(next_obj(begin), end, fetch_count - 1);
  }
  return begin;
}

void *ThreadCache::allocate(size_t bytes) {
  if (bytes > MAX_BYTES) {
    // 直接向Central Cache申请
    // TODO
  }

  size_t size = Utility::round_up(bytes);
  int idx = Utility::index(size);

  if (!free_lists_[idx].empty()) {
    return free_lists_[idx].pop();
  } else {
    // 向Central Cache申请内存块
    return fetch_from_centralcache(idx, size);
  }
}

// 释放内存
void ThreadCache::deallocate(void *obj, size_t size) {
  assert(obj != nullptr);
  int idx = Utility::index(size);

  FreeList &list = free_lists_[idx];
  list.push(obj);

  if (list.length() >= list.max_length()) {
    list_too_long(list, size);
  }
}

void ThreadCache::list_too_long(FreeList &list, size_t size) {
  void *begin = nullptr;
  void *end = nullptr;
  // 将内存块取下maxLength个
  list.pop_range(begin, end, list.max_length());

  // 将这些内存块返还给Central Cache
  CentralCache::get_instance()->release_list_to_spans(begin, size);
}

ThreadCache::~ThreadCache() {
  for (auto &list : free_lists_) {
    if (!list.empty()) {
      CentralCache::get_instance()->release_list_to_spans(list.head(),
                                                          list.size());
    }
  }
}