#define _CRT_SECURE_NO_WARNINGS 1
#include "FreeList.h"
inline void *&NextObj(void *obj) { return *(void **)obj; }

void FreeList::Push(void *obj) // 头插一个节点
{
  assert(obj);
  _length++;
  NextObj(obj) = _head;
  _head = obj;
}

void *FreeList::Pop() // 取下头结点
{
  assert(_head != nullptr);
  assert(_length > 0);
  _length--;
  void *obj = _head;
  _head = NextObj(_head);
  return obj;
}

void FreeList::PushRange(void *begin, void *end, size_t n) {
  // 将空闲链表 begin->...->end 头插至本链表
  NextObj(end) = _head;
  _head = begin;
  _length += n;
}

void FreeList::PopRange(void *&begin, void *&end, size_t n) {
  assert(n <= _length);
  begin = _head;
  end = begin;
  for (size_t i = 0; i < n - 1; ++i) {
    end = NextObj(end);
  }
  _head = NextObj(end);
  NextObj(end) = nullptr;
  _length -= n;
}