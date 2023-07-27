#include "FreeList.h"
#include "Utility.h"

void FreeList::push(void *obj) // 头插一个节点
{
  assert(obj);
  length_++;
  next_obj(obj) = head_;
  head_ = obj;
}

void *FreeList::pop() // 取下头结点
{
  assert(head_ != nullptr);
  assert(length_ > 0);
  length_--;
  void *obj = head_;
  head_ = next_obj(head_);
  return obj;
}

void FreeList::push_range(void *begin, void *end, size_t n) {
  // 将空闲链表 begin->...->end 头插至本链表
  next_obj(end) = head_;
  head_ = begin;
  length_ += n;
}

void FreeList::pop_range(void *&begin, void *&end, size_t n) {
  assert(n <= length_);
  begin = head_;
  end = begin;
  for (size_t i = 0; i < n - 1; ++i) {
    end = next_obj(end);
  }
  head_ = next_obj(end);
  next_obj(end) = nullptr;
  length_ -= n;
}