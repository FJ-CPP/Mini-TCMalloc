#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <vector>
#include <ctime>
#include <thread>
#include "TCMalloc.h"
#include "ObjectPool.hpp"

/*
 * ��׼�����ļ�
*/

/*
 * ntimes: ���ִ������ͷŴ���
 * nworks: �߳���
 * rounds: ִ���ִ�
*/
void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	std::atomic<size_t> malloc_costtime = 0;
	std::atomic<size_t> free_costtime = 0;

	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			std::vector<void*> v;

			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					//v.push_back(malloc((i * j + 1) % 8192 + 1));
					v.push_back(malloc(10));
				}
				size_t end1 = clock();

				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					free(v[i]);
				}
				size_t end2 = clock();
				v.clear();

				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}

	for (auto& t : vthread)
	{
		t.join();
	}

	std::cout << nworks << "���̲߳���ִ��" << rounds << "�Σ�ÿ��malloc " << ntimes << "�Σ���ʱ��" << malloc_costtime << std::endl;

	std::cout << nworks << "���̲߳���ִ��" << rounds << "�Σ�ÿ��free " << ntimes << "�Σ���ʱ��" << free_costtime << std::endl;

	std::cout << "�ܼƺ�ʱ:" << malloc_costtime + free_costtime << std::endl;
}

/*
 * ntimes: ���ִ������ͷŴ���
 * nworks: �߳���
 * rounds: ִ���ִ�
*/
void BenchmarkTCMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	std::atomic<size_t> malloc_costtime = 0;
	std::atomic<size_t> free_costtime = 0;

	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			std::vector<void*> v;

			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					//v.push_back(TCMalloc((i * j + 1) % 8192 + 1));
					v.push_back(TCMalloc(10));
				}
				size_t end1 = clock();

				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					TCFree(v[i]);
				}
				size_t end2 = clock();
				v.clear();

				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}

	for (auto& t : vthread)
	{
		t.join();
	}

	std::cout << nworks << "���̲߳���ִ��" << rounds << "�Σ�ÿ��tcmalloc " << ntimes << "�Σ���ʱ��" << malloc_costtime << std::endl;

	std::cout << nworks << "���̲߳���ִ��" << rounds << "�Σ�ÿ��tcfree " << ntimes << "�Σ���ʱ��" << free_costtime << std::endl;

	std::cout << "�ܼƺ�ʱ��" << malloc_costtime + free_costtime << std::endl;
}

int main()
{
	srand((unsigned int)time(0));
	size_t times = 10000;
	int nthread = 10;
	size_t n = 10;

	while (1)
	{
		std::cout << "========================= begin ========================" << std::endl;
		BenchmarkTCMalloc(times, nthread, n);
		std::cout << std::endl << std::endl;
		BenchmarkMalloc(times, nthread, n);
		std::cout << "=========================  end  ========================" << std::endl << std::endl;
	}
}