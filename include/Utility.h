#pragma once
#include "common.h"

// 内存对齐规则和哈希桶下标计算
class Utility {
private:
  static size_t RoundUpHelper(size_t bytes, int alignNum);

  static size_t IndexHelper(size_t bytes, size_t alignShift);

public:
  // 将size向上对齐
  static size_t RoundUp(size_t bytes);

  // 获取对齐后大小为size的内存块在哈希表中的下标
  static int Index(size_t size);

  static size_t NumMoveSize(size_t size);

  // 根据size计算其应当由npage页的Span分割而来
  static size_t NumMovePage(size_t size);
};