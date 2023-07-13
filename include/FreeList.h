#pragma once
#include "common.h"

// 空闲链表
class FreeList {
private:
  void *head_ = nullptr;
  size_t size_ = 0;       // 当前链表内的内存块大小
  size_t length_ = 0;     // 当前链表的长度
  size_t max_length_ = 1; // 当前链表的最大长度(慢启动阈值)
public:
  // 头插一个节点
  void push(void *obj);

  // 取下头结点
  void *pop();

  // 头插n个结点
  void push_range(void *begin, void *end, size_t n);

  // 头删n个结点(输出型参数)
  void pop_range(void *&begin, void *&end, size_t n);

  // 获取头结点
  void *head() const { return head_; }

  // 判断是否为空
  bool empty() const {
    return head_ == nullptr;
    // return length_ == 0;
  }

  // 获取链表内存储的内存块大小
  size_t size() const { return size_; }

  // 设置链表内存储的内存块大小
  void set_size(size_t size) { size_ = size; }

  // 获取当前链表长度
  size_t length() const { return length_; }

  // 获取当前链表长度阈值
  size_t max_length() const { return max_length_; }

  // 设置当前链表长度阈值
  void set_max_length(size_t newMax) { max_length_ = newMax; }
};