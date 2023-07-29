#include "SpanList.h"
#include <gtest/gtest.h>

class SpanListTest : public ::testing::Test {
protected:
  SpanList *spanList;

  void SetUp() override { spanList = new SpanList(); }

  void TearDown() override { delete spanList; }
};

// Test Case 1: 测试新创建的SpanList是否为空
TEST_F(SpanListTest, IsEmptyOnNewList) { EXPECT_TRUE(spanList->empty()); }

// Test Case 2: 检查新列表的begin()和end()
TEST_F(SpanListTest, BeginEqualsEndOnNewList) {
  EXPECT_EQ(spanList->begin(), spanList->end());
}

// Test Case 3: 在空列表上push_front
TEST_F(SpanListTest, PushFrontOnEmptyList) {
  Span *span = new Span;
  spanList->push_front(span);
  EXPECT_FALSE(spanList->empty());
}

// Test Case 4: 在空列表上pop_front
TEST_F(SpanListTest, PopFrontOnEmptyList) {
  EXPECT_THROW(spanList->pop_front(), std::runtime_error);
}

// Test Case 5: 添加元素后检查begin()和end()
TEST_F(SpanListTest, BeginNotEqualsEndAfterPushFront) {
  Span *span = new Span;
  spanList->push_front(span);
  EXPECT_NE(spanList->begin(), spanList->end());
}

// Test Case 6: 添加元素并从列表中删除
TEST_F(SpanListTest, RemoveElementAfterPushFront) {
  Span *span = new Span;
  spanList->push_front(span);
  Span *poppedSpan = spanList->pop_front();
  EXPECT_EQ(poppedSpan, span);
  EXPECT_TRUE(spanList->empty());
}

// Test Case 7: 插入元素到指定位置
TEST_F(SpanListTest, InsertElementAtPosition) {
  Span *span1 = new Span;
  Span *span2 = new Span;
  spanList->push_front(span1);
  spanList->insert(spanList->begin(), span2);
  EXPECT_EQ(spanList->begin(), span2);
}

// Test Case 8: 测试erase函数
TEST_F(SpanListTest, EraseElement) {
  Span *span = new Span;
  spanList->push_front(span);
  spanList->erase(spanList->begin());
  EXPECT_TRUE(spanList->empty());
}

// Test Case 9: 测试添加和删除多个元素
TEST_F(SpanListTest, MultiplePushAndPop) {
  const int N = 100;
  for (int i = 0; i < N; ++i) {
    Span *span = new Span;
    span->page_id = i;
    spanList->push_front(span);
  }

  EXPECT_FALSE(spanList->empty());

  for (int i = 0; i < N; ++i) {
    Span *poppedSpan = spanList->pop_front();
    EXPECT_EQ(poppedSpan->page_id, N - i - 1);
  }

  EXPECT_TRUE(spanList->empty());
}

// // Test Case 10: 对同一元素重复push_front
// TEST_F(SpanListTest, PushSameElementTwice) {
//   Span *span = new Span;
//   spanList->push_front(span);
//   EXPECT_THROW(spanList->push_front(span), std::runtime_error);
// }

// // Test Case 11: 对同一元素重复insert
// TEST_F(SpanListTest, InsertSameElementTwice) {
//   Span *span1 = new Span;
//   Span *span2 = new Span;
//   spanList->push_front(span1);
//   EXPECT_THROW(spanList->insert(spanList->begin(), span1),
//   std::runtime_error);
// }

// // Test Case 12: erase不存在的元素
// TEST_F(SpanListTest, EraseNonexistentElement) {
//   Span *span = new Span;
//   EXPECT_THROW(spanList->erase(span), std::runtime_error);
// }

// // Test Case 13: insert到不存在的位置
// TEST_F(SpanListTest, InsertAtNonexistentPosition) {
//   Span *span1 = new Span;
//   Span *span2 = new Span;
//   EXPECT_THROW(spanList->insert(span1, span2), std::runtime_error);
// }

// Test Case 14: push_front nullptr
TEST_F(SpanListTest, PushFrontNullptr) {
  EXPECT_THROW(spanList->push_front(nullptr), std::runtime_error);
}

// Test Case 15: insert nullptr
TEST_F(SpanListTest, InsertNullptr) {
  Span *span = new Span;
  spanList->push_front(span);
  EXPECT_THROW(spanList->insert(spanList->begin(), nullptr),
               std::runtime_error);
}

// Test Case 16: erase nullptr
TEST_F(SpanListTest, EraseNullptr) {
  EXPECT_THROW(spanList->erase(nullptr), std::runtime_error);
}

// // Test Case 17: insert到end()位置
// TEST_F(SpanListTest, InsertAtEnd) {
//   Span *span = new Span;
//   EXPECT_THROW(spanList->insert(spanList->end(), span), std::runtime_error);
// }

// Test Case 18: 确保列表在析构时不会泄露内存
TEST_F(SpanListTest, NoMemoryLeakOnDestruction) {
  const int N = 100000;
  for (int i = 0; i < N; ++i) {
    Span *span = new Span;
    span->page_id = i;
    spanList->push_front(span);
  }
  // 此处无需进行任何操作，因为随着测试框架调用TearDown()，spanList将被正确地销毁。
}

// Test Case 19: 对空列表进行erase操作
TEST_F(SpanListTest, EraseEmptyList) {
  EXPECT_THROW(spanList->erase(spanList->begin()), std::runtime_error);
}

// Test Case 20: 在已经push_front元素的列表中再次调用begin()和end()
TEST_F(SpanListTest, BeginEndAfterInsertions) {
  Span *span1 = new Span;
  Span *span2 = new Span;
  spanList->push_front(span1);
  spanList->push_front(span2);

  EXPECT_NE(spanList->begin(), spanList->end());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
