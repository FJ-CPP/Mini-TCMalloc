project(mini-tcmalloc)

cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_CXX_STANDARD 14)

option(BUILD_TEST "Build test files." ON)

option(BUILD_TUTORIALS "Build tutorial files." ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "release")
endif(NOT CMAKE_BUILD_TYPE)

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if(NOT CMAKE_BUILD_TYPE MATCHES "debug")
  message(WARNING "Building release!!!")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -O3 -Wall -fvisibility-inlines-hidden"
  )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -fvisibility-inlines-hidden")
else()
  message(WARNING "Building debug!!!")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3 -Wall")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3 -Wall")
endif()

set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")

set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")

set(MINI_TCMALLOC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-static-libstdc++)

add_subdirectory(include)

add_subdirectory(src)

if(BUILD_TEST)
  # message(STATUS "Build gtest!!!") add_subdirectory(thirdparty/googletest)
  message(STATUS "Build test files!!!")
  add_subdirectory(test)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif(BUILD_TEST)

if(BUILD_TUTORIALS)
  message(STATUS "Build tutorial files!!!")
  add_subdirectory(tutorial)
endif(BUILD_TUTORIALS)
