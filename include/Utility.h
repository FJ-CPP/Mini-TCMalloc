#pragma once
#include "common.h"

// 内存对齐规则和哈希桶下标计算
class Utility {
private:
  static size_t roundup_helper(size_t bytes, int alignNum);

  static size_t index_helper(size_t bytes, size_t alignShift);

public:
  // 将size向上对齐
  static size_t round_up(size_t bytes);

  // 获取对齐后大小为size的内存块在哈希表中的下标
  static int index(size_t size);

  static size_t num_move_size(size_t size);

  // 根据size计算其应当由npage页的Span分割而来
  static size_t num_move_page(size_t size);
};