#pragma once
#include "common.h"

// 空闲链表
class FreeList {
private:
  void *_head = nullptr;
  size_t _size = 0;      // 当前链表内的内存块大小
  size_t _length = 0;    // 当前链表的长度
  size_t _maxLength = 1; // 当前链表的最大长度(慢启动阈值)
public:
  // 头插一个节点
  void Push(void *obj);

  // 取下头结点
  void *Pop();

  // 头插n个结点
  void PushRange(void *begin, void *end, size_t n);

  // 头删n个结点(输出型参数)
  void PopRange(void *&begin, void *&end, size_t n);

  // 获取头结点
  void *Head() const { return _head; }

  // 判断是否为空
  bool Empty() const {
    return _head == nullptr;
    // return _length == 0;
  }

  // 获取链表内存储的内存块大小
  size_t Size() const { return _size; }

  // 设置链表内存储的内存块大小
  void SetSize(size_t size) { _size = size; }

  // 获取当前链表长度
  size_t Length() const { return _length; }

  // 获取当前链表长度阈值
  size_t MaxLength() const { return _maxLength; }

  // 设置当前链表长度阈值
  void SetMaxLength(size_t newMax) { _maxLength = newMax; }
};