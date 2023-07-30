#include "SpanList.h"
#include "ObjectPool.hpp"
#include "Static.h"

SpanList::SpanList() {
  Static::static_span_pool.lock();
  head_ = Static::static_span_pool.New();
  Static::static_span_pool.unlock();
  head_->next = head_;
  head_->prev = head_;
}

SpanList::~SpanList() {
  Static::static_span_pool.lock();
  Static::static_span_pool.Delete(head_);
  Static::static_span_pool.unlock();
}

void SpanList::push_front(Span *new_span) {
  ASSERT(new_span);
  insert(begin(), new_span);
}

Span *SpanList::pop_front() {
  ASSERT(!empty());
  Span *front = begin();
  erase(front);
  return front;
}

void SpanList::insert(Span *pos, Span *new_span) {
  ASSERT(new_span);
  ASSERT(pos);

  Span *prev = pos->prev;

  prev->next = new_span;
  new_span->prev = prev;
  new_span->next = pos;
  pos->prev = new_span;
}

void SpanList::erase(Span *pos) {
  ASSERT(pos);
  ASSERT(pos != head_);

  Span *prev = pos->prev;
  Span *next = pos->next;

  prev->next = next;
  next->prev = prev;
}