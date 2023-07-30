#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include "TCMalloc.h"
#include "ObjectPool.hpp"

/*
 * 基准测试
 */

/*
 * ntimes: 单轮次申请释放次数
 * nworks: 线程数
 * rounds: 执行轮次
 */
static const int ALLOC_SIZE_MAX = 8192;

void benchmark_malloc(size_t ntimes, size_t nworks, size_t rounds) {
  std::vector<std::thread> vthread(nworks);
  std::atomic<size_t> malloc_costtime(0);
  std::atomic<size_t> free_costtime(0);

  for (size_t k = 0; k < nworks; ++k) {
    vthread[k] = std::thread([&]() {
      std::vector<void *> v;

      for (size_t j = 0; j < rounds; ++j) {
        auto begin1 = std::chrono::steady_clock::now().time_since_epoch();
        for (size_t i = 0; i < ntimes; i++) {
          v.push_back(malloc((i * j + 1) % ALLOC_SIZE_MAX + 1));
          // v.push_back(malloc(10));
        }
        auto end1 = std::chrono::steady_clock::now().time_since_epoch();

        auto begin2 = std::chrono::steady_clock::now().time_since_epoch();
        for (size_t i = 0; i < ntimes; i++) {
          free(v[i]);
        }
        auto end2 = std::chrono::steady_clock::now().time_since_epoch();
        v.clear();

        malloc_costtime +=
            std::chrono::duration_cast<std::chrono::milliseconds>(end1 - begin1)
                .count();
        free_costtime +=
            std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin2)
                .count();
      }
    });
  }

  for (auto &t : vthread) {
    t.join();
  }

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮malloc "
            << ntimes << "次, 耗时：" << malloc_costtime << "ms" << std::endl;

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮free " << ntimes
            << "次, 耗时：" << free_costtime << "ms" << std::endl;

  std::cout << "总计耗时:" << malloc_costtime + free_costtime << "ms"
            << std::endl;
}

/*
 * ntimes: 单轮次申请释放次数
 * nworks: 线程数
 * rounds: 执行轮次
 */
void benchmark_tcmalloc(size_t ntimes, size_t nworks, size_t rounds) {
  std::vector<std::thread> vthread(nworks);
  std::atomic<size_t> malloc_costtime(0);
  std::atomic<size_t> free_costtime(0);

  for (size_t k = 0; k < nworks; ++k) {
    vthread[k] = std::thread([&]() {
      std::vector<void *> v;

      for (size_t j = 0; j < rounds; ++j) {
        auto begin1 = std::chrono::steady_clock::now().time_since_epoch();
        for (size_t i = 0; i < ntimes; i++) {
          v.push_back(tcmalloc((i * j + 1) % ALLOC_SIZE_MAX + 1));
          // v.push_back(tcmalloc(10));
        }
        auto end1 = std::chrono::steady_clock::now().time_since_epoch();

        auto begin2 = std::chrono::steady_clock::now().time_since_epoch();
        for (size_t i = 0; i < ntimes; i++) {
          tcfree(v[i]);
        }
        auto end2 = std::chrono::steady_clock::now().time_since_epoch();
        v.clear();

        malloc_costtime +=
            std::chrono::duration_cast<std::chrono::milliseconds>(end1 - begin1)
                .count();
        free_costtime +=
            std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin2)
                .count();
      }
    });
  }

  for (auto &t : vthread) {
    t.join();
  }

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮tcmalloc "
            << ntimes << "次, 耗时：" << malloc_costtime << "ms" << std::endl;

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮tcfree "
            << ntimes << "次, 耗时：" << free_costtime << "ms" << std::endl;

  std::cout << "总计耗时：" << malloc_costtime + free_costtime << "ms"
            << std::endl;
}

int main(int argc, const char *argv[]) {
  if (argc != 5) {
    std::cout << "Usage:\n\t" << argv[0]
              << " [thread_num] [rounds] [malloc_times] [repeat_time]\n";
    std::abort();
  }

  srand((unsigned int)time(0));
  int nthread = atoi(argv[1]);
  size_t n = atoi(argv[2]);
  size_t times = atoi(argv[3]);
  int repeat_time = atoi(argv[4]);

  // while (1) {
  //   std::cout << "tcmalloc" << std::endl;
  //   std::vector<void *> v;
  //   for (int i = 0; i < 500; ++i) {
  //     int *p = (int *)tcmalloc(129 * 1024 * 8);
  //     v.emplace_back(p);
  //     for (int j = 0; j < 129 * 1024 * 2; ++j) {
  //       p[j] = 0;
  //     }
  //   }
  //   for (auto e : v) {
  //     tcfree(e);
  //   }
  //   std::this_thread::sleep_for(std::chrono::seconds(1));
  // }

  while (repeat_time--) {
    std::cout << "========================= begin ========================"
              << std::endl;
    benchmark_tcmalloc(times, nthread, n);
    std::cout << std::endl << std::endl;
    benchmark_malloc(times, nthread, n);
    std::cout << "=========================  end  ========================"
              << std::endl
              << std::endl;
  }

  return 0;
}