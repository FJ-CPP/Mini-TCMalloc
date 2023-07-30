#pragma once

#include "SpanList.h"
#include "ObjectPool.hpp"
#include <unordered_map>

class Static {
public:
  static std::unordered_map<void *, int> kpage_map;

  static ObjectPool<Span> static_span_pool;
};
