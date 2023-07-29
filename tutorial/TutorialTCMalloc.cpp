#include "TCMalloc.h"
#include <iostream>

int main() {
  const int N = 10;
  int *ptr = reinterpret_cast<int *>(tcmalloc(N * sizeof(int)));

  for (int i = 0; i < N; ++i) {
    ptr[i] = i;
  }

  for (int i = 0; i < N; ++i) {
    std::cout << ptr[i] << " ";
  }
  std::cout << std::endl;

  tcfree(ptr);

  return 0;
}