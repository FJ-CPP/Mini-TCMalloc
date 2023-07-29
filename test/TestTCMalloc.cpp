#include "TCMalloc.h"
#include <gtest/gtest.h>
#include <climits>

TEST(TCMallocTest, NormalAllocationFree) {
  void *my_memory = tcmalloc(64);
  EXPECT_NE(my_memory, nullptr);
  tcfree(my_memory);
}

TEST(TCMallocTest, ZeroByteAllocation) {
  void *zero_byte_memory = tcmalloc(0);
  EXPECT_EQ(zero_byte_memory, nullptr);
  tcfree(zero_byte_memory);
}

TEST(TCMallocTest, MultipleAllocationsAndFrees) {
  for (int i = 0; i < 10000; ++i) {
    void *loop_memory = tcmalloc(1024);
    EXPECT_NE(loop_memory, nullptr);
    tcfree(loop_memory);
  }
}

// These tests might cause errors depending on the implementation of TCMalloc.
TEST(TCMallocTest, FreeingNonTCMallocMemory) {
  int *memory_not_from_tcmalloc = new int[10];
  EXPECT_DEATH(tcfree(memory_not_from_tcmalloc), "");
}

// TEST(TCMallocTest, DoubleFree) {
//   void *double_free_memory = tcmalloc(128);
//   EXPECT_NE(double_free_memory, nullptr);
//   tcfree(double_free_memory);
//   EXPECT_DEATH(tcfree(double_free_memory), "");
// }

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
