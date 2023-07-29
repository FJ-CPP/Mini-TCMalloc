#include "Utility.h"

size_t Utility::roundup_helper(size_t bytes, int align_num) {
  // if (size % align_num == 0)
  //{
  //	return size;
  //}
  // return (size / align_num + 1) * align_num;

  // 下面这行代码等价于上面注释后的代码
  // 经测试，二者性能相差无几，不过下面这行更加的优美 :)
  return ((bytes + align_num - 1) & ~(align_num - 1));
}

size_t Utility::index_helper(size_t bytes, size_t align_shift) {
  // 这里 1 << align_shift == align_num
  // 依然是优美的代码 :(
  return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
}

size_t Utility::round_up(size_t bytes) {
  if (bytes <= 128) {
    return roundup_helper(bytes, 8);
  } else if (bytes <= 1024) {
    return roundup_helper(bytes, 16);
  } else if (bytes <= 8 * 1024) {
    return roundup_helper(bytes, 128);
  } else if (bytes <= 64 * 1024) {
    return roundup_helper(bytes, 1024);
  } else if (bytes <= 256 * 1024) {
    return roundup_helper(bytes, 8 * 1024);
  } else {
    // 大于256kb的按照页大小进行对齐
    return roundup_helper(bytes, 1 << PAGE_SHIFT);
  }
}

int Utility::index(size_t size) {
  static int count[4] = {16, 56, 56, 56};
  if (size <= 128) {
    return index_helper(size, 3);
  } else if (size <= 1024) {
    return index_helper(size - 128, 4) + count[0];
  } else if (size <= 8 * 1024) {
    return index_helper(size - 1024, 7) + count[1] + count[0];
  } else if (size <= 64 * 1024) {
    return index_helper(size - 8 * 1024, 10) + count[2] + count[1] + count[0];
  } else if (size <= 256 * 1024) {
    return index_helper(size - 64 * 1024, 13) + count[3] + count[2] + count[1] +
           count[0];
  } else {
    ASSERT(false);
    return -1;
  }
}

size_t Utility::num_move_size(size_t size) {
  ASSERT(size > 0);

  // 慢启动的阈值
  // 小对象一次最多申请512个
  // 大对象一次至少申请2个

  int num = MAX_BYTES / size;

  if (num < 2) {
    num = 2;
  } else if (num > 512) {
    num = 512;
  }

  return num;
}

size_t Utility::num_move_page(size_t size) {
  size_t num = num_move_size(size);
  size_t npage = num * size;

  npage >>= PAGE_SHIFT;
  if (npage == 0) {
    npage = 1;
  }

  return npage;
}