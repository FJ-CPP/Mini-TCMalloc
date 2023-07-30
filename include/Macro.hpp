#pragma once

#include <cassert>
#include <stdexcept>

enum LogLevel { DEBUG, INFO, WARNN, ERROR, FATAL };

#include <cstdio>
#define LOG(LV, MSG)                                                           \
  fprintf(stderr, "[%s] [%s:%d] [%s]\n", #LV, __FILE__, __LINE__, (MSG));

#define ASSERT(COND)                                                           \
  if (!(COND)) {                                                               \
    LOG(FATAL, #COND)                                                          \
    throw std::runtime_error(#COND);                                           \
  }
