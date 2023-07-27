#include <iostream>
#include <vector>
#include <ctime>
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

void benchmarl_malloc(size_t ntimes, size_t nworks, size_t rounds) {
  std::vector<std::thread> vthread(nworks);
  std::atomic<size_t> malloc_costtime(0);
  std::atomic<size_t> free_costtime(0);

  for (size_t k = 0; k < nworks; ++k) {
    vthread[k] = std::thread([&]() {
      std::vector<void *> v;

      for (size_t j = 0; j < rounds; ++j) {
        size_t begin1 = clock();
        for (size_t i = 0; i < ntimes; i++) {
          v.push_back(malloc((i * j + 1) % ALLOC_SIZE_MAX + 1));
          // v.push_back(malloc(10));
        }
        size_t end1 = clock();

        size_t begin2 = clock();
        for (size_t i = 0; i < ntimes; i++) {
          free(v[i]);
        }
        size_t end2 = clock();
        v.clear();

        malloc_costtime += (end1 - begin1);
        free_costtime += (end2 - begin2);
      }
    });
  }

  for (auto &t : vthread) {
    t.join();
  }

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮malloc "
            << ntimes << "次, 耗时：" << malloc_costtime << std::endl;

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮free " << ntimes
            << "次, 耗时：" << free_costtime << std::endl;

  std::cout << "总计耗时:" << malloc_costtime + free_costtime << std::endl;
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
        size_t begin1 = clock();
        for (size_t i = 0; i < ntimes; i++) {
          v.push_back(tcmalloc((i * j + 1) % ALLOC_SIZE_MAX + 1));
          // v.push_back(tcmalloc(10));
        }
        size_t end1 = clock();

        size_t begin2 = clock();
        for (size_t i = 0; i < ntimes; i++) {
          tcfree(v[i]);
        }
        size_t end2 = clock();
        v.clear();

        malloc_costtime += (end1 - begin1);
        free_costtime += (end2 - begin2);
      }
    });
  }

  for (auto &t : vthread) {
    t.join();
  }

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮tcmalloc "
            << ntimes << "次, 耗时：" << malloc_costtime << std::endl;

  std::cout << nworks << "个线程并发执行" << rounds << "次, 每轮tcfree "
            << ntimes << "次, 耗时：" << free_costtime << std::endl;

  std::cout << "总计耗时：" << malloc_costtime + free_costtime << std::endl;
}

int main() {
  srand((unsigned int)time(0));
  size_t times = 10000;
  int nthread = 10;
  size_t n = 10;

  std::vector<void *> v;
  for (int i = 0; i < 1000; ++i) {
    v.emplace_back(tcmalloc(129 * 1024 * 8));
  }
  for (auto e : v) {
    tcfree(e);
  }

  while (1) {
    std::cout << "========================= begin ========================"
              << std::endl;
    benchmark_tcmalloc(times, nthread, n);
    std::cout << std::endl << std::endl;
    benchmarl_malloc(times, nthread, n);
    std::cout << "=========================  end  ========================"
              << std::endl
              << std::endl;
  }
}