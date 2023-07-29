#include "ObjectPool.hpp"
#include <gtest/gtest.h>
#include <thread>

struct MyStruct {
  int a;
  double b;
};

class ObjectPoolTest : public ::testing::Test {
protected:
  ObjectPool<MyStruct> *pool;

  void SetUp() override { pool = new ObjectPool<MyStruct>(); }

  void TearDown() override { delete pool; }
};

// Test Case 1: 测试能否正常创建对象
TEST_F(ObjectPoolTest, CreateObject) {
  MyStruct *obj = pool->New();
  EXPECT_NE(obj, nullptr);
}

// Test Case 2: 创建的对象能否正确赋值
TEST_F(ObjectPoolTest, AssignToObject) {
  MyStruct *obj = pool->New();
  obj->a = 5;
  obj->b = 3.14;
  EXPECT_EQ(5, obj->a);
  EXPECT_DOUBLE_EQ(3.14, obj->b);
}

// Test Case 3: 是否能正常删除对象
TEST_F(ObjectPoolTest, DeleteObject) {
  MyStruct *obj = pool->New();
  pool->Delete(obj);
  // Note: After deletion, we cannot use ASSERT_* or EXPECT_* to check its
  // value.
}

// Test Case 4: 测试多次创建和删除对象
TEST_F(ObjectPoolTest, CreateDeleteMultipleObjects) {
  MyStruct *obj1 = pool->New();
  pool->Delete(obj1);

  MyStruct *obj2 = pool->New();
  EXPECT_EQ(obj1, obj2); // 因为内存池复用内存，所以这两个对象的地址应该是一样的

  pool->Delete(obj2);
}

// Test Case 5: 测试创建大量对象
TEST_F(ObjectPoolTest, CreateManyObjects) {
  std::vector<MyStruct *> objects;

  for (int i = 0; i < 10000; ++i) {
    MyStruct *obj = pool->New();
    obj->a = i;
    obj->b = i * 0.1;
    objects.push_back(obj);
  }

  for (int i = 0; i < 10000; ++i) {
    EXPECT_EQ(i, objects[i]->a);
    EXPECT_DOUBLE_EQ(i * 0.1, objects[i]->b);
  }

  // Don't forget to delete all the objects
  for (MyStruct *obj : objects) {
    pool->Delete(obj);
  }
}

// Test Case 6: 在没有调用 New() 的情况下调用 Delete()
TEST_F(ObjectPoolTest, DeleteWithoutNew) {
  EXPECT_THROW(pool->Delete(nullptr), std::runtime_error);
}

// Test Case 7: 多线程同时访问对象池
TEST_F(ObjectPoolTest, MultithreadedAccess) {
  std::vector<std::thread> threads;

  for (int i = 0; i < 10; ++i) {
    threads.push_back(std::thread([this]() {
      MyStruct *obj = pool->New();
      pool->Delete(obj);
    }));
  }

  for (std::thread &t : threads) {
    t.join();
  }
}

// Test Case 8: 测试lock和unlock函数
TEST_F(ObjectPoolTest, LockUnlock) {
  pool->lock();
  // 在这里不能有任何会抛出异常的操作，否则可能导致死锁
  pool->unlock();
}

// Test Case 9: 再次测试空指针删除
TEST_F(ObjectPoolTest, DeleteNullptrAgain) {
  EXPECT_THROW(pool->Delete(nullptr), std::runtime_error);
}

// Test Case 10: 测试分配超过内存池大小的对象
TEST_F(ObjectPoolTest, AllocateLargeObject) {
  ObjectPool<char[1024 * 1024]> largePool; // 1 MB 对象的池
  EXPECT_NO_THROW(largePool.New());
}

// // Test Case 11: 测试相同对象是否能被删除两次
// TEST_F(ObjectPoolTest, DeleteSameObjectTwice) {
//   MyStruct *obj = pool->New();
//   pool->Delete(obj);
//   EXPECT_THROW(
//       pool->Delete(obj),
//       std::runtime_error); // The same object should not be deleted twice.
// }

// Test Case 12: 创建并删除对象后，创建新对象应该使用刚释放的内存
TEST_F(ObjectPoolTest, ReuseMemoryAfterDeletion) {
  MyStruct *obj1 = pool->New();
  pool->Delete(obj1);

  MyStruct *obj2 = pool->New();
  EXPECT_EQ(obj1, obj2); // 'obj2' should reuse the memory of 'obj1'.
  pool->Delete(obj2);
}

// Test Case 13: 测试多次调用 New() 和 Delete()
TEST_F(ObjectPoolTest, MultipleNewDelete) {
  for (int i = 0; i < 100; ++i) {
    MyStruct *obj = pool->New();
    pool->Delete(obj);
  }
  // No check here, if something is wrong an exception will be thrown.
}

// Test Case 14: 多线程同时创建和删除大量对象
TEST_F(ObjectPoolTest, MultithreadedManyObjects) {
  std::vector<std::thread> threads;

  for (int i = 0; i < 10; ++i) {
    threads.push_back(std::thread([this]() {
      for (int j = 0; j < 100; ++j) {
        MyStruct *obj = pool->New();
        pool->Delete(obj);
      }
    }));
  }

  for (std::thread &t : threads) {
    t.join();
  }
}

// Test Case 15: 创建数量大于内存池初始大小的对象
TEST_F(ObjectPoolTest, CreateMoreObjectsThanInitialSize) {
  for (int i = 0; i < 20000;
       ++i) { // Assuming initial size can contain less than 20000 objects.
    EXPECT_NO_THROW(pool->New());
  }
}

// Test Case 16: 在没有创建任何对象的情况下检查free_list_是否为空
TEST_F(ObjectPoolTest, CheckFreeListWithoutNew) {
  // We cannot directly access free_list_, so we test this indirectly.
  for (int i = 0; i < 100; ++i) {
    pool->Delete(new MyStruct);
  }
  MyStruct *obj = pool->New(); // This should not throw any exception.
  pool->Delete(obj);
}

// Test Case 17: 锁定对象池时尝试删除对象
TEST_F(ObjectPoolTest, DeleteWhileLocked) {
  MyStruct *obj = pool->New();
  pool->lock();
  // In current implementation this doesn't cause a deadlock, but in some other
  // implementations it might.
  EXPECT_NO_THROW(pool->Delete(obj));
  pool->unlock();
}

// Test Case 18: 测试在析构函数中释放已使用缓冲区
TEST_F(ObjectPoolTest, DestructionReleasesUsedBuffer) {
  // No actual check here, just make sure no exception is thrown during
  // destruction.
}

// // Test Case 19: 创建一个特定大小的对象池
// TEST_F(ObjectPoolTest, CreateSpecificSizedObjectPool) {
//   ObjectPool<char[256]> specificSizedPool; // Object pool for 256-byte
//   objects. char(*arr)[256] = specificSizedPool.New(); EXPECT_NE(arr,
//   nullptr); specificSizedPool.Delete(arr);
// }

// Test Case 20: 多次调用 lock() 和 unlock()
TEST_F(ObjectPoolTest, MultipleLockUnlock) {
  for (int i = 0; i < 100; ++i) {
    EXPECT_NO_THROW(pool->lock());
    EXPECT_NO_THROW(pool->unlock());
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
