#define _CRT_SECURE_NO_WARNINGS 1
#include "SpanList.h"

void SpanList::PushFront(Span *newSpan) { Insert(Begin(), newSpan); }

Span *SpanList::PopFront() {
  assert(!Empty());
  Span *front = Begin();
  Erase(front);
  return front;
}

void SpanList::Insert(Span *pos, Span *newSpan) {
  assert(newSpan);
  assert(pos);

  Span *prev = pos->_prev;

  prev->_next = newSpan;
  newSpan->_prev = prev;
  newSpan->_next = pos;
  pos->_prev = newSpan;
}

void SpanList::Erase(Span *pos) {
  assert(pos);
  assert(pos != _head);

  Span *prev = pos->_prev;
  Span *next = pos->_next;

  prev->_next = next;
  next->_prev = prev;
}