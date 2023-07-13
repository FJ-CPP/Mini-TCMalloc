#pragma once
#include "common.h"
#include <mutex>

struct Span {
  PAGE_ID page_id = 0; // Span的起始页页号
  size_t obj_size = 0; // Span切出去的小内存块的大小
  size_t n = 0;        // 页面的数量
  int use_count = 0; // Span被分割成小内存块后，有多少个已经被使用

  void *free_list = nullptr; // 管理小内存块的空闲链表

  Span *prev = nullptr;
  Span *next = nullptr;

  bool is_used = false;
};

// 带头双向链表管理Span
class SpanList {
private:
  Span *head_;
  std::mutex mtx_;

public:
  SpanList() {
    head_ = new Span;
    head_->next = head_;
    head_->prev = head_;
  }

  bool empty() { return head_->next == head_; }

  Span *begin() { return head_->next; }

  Span *end() { return head_; }

  void push_front(Span *newSpan);

  Span *pop_front();

  void insert(Span *pos, Span *newSpan);

  void erase(Span *pos);

  void lock() { mtx_.lock(); }

  void unlock() { mtx_.unlock(); }
};