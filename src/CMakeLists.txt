option(USE_STATIC_MINI_TCMALLOC_LIBRARY "Build mini-tcmalloc static library."
       OFF
)
option(USE_SHARED_MINI_TCMALLOC_LIBRARY "Build mini-tcmalloc shared library."
       ON
)

file(GLOB src_files "*.cpp")

set(lib_static_target "static-mini-tcmalloc")
set(lib_shared_target "mini-tcmalloc")

set(mini_tcmalloc_lib "")

if(USE_STATIC_MINI_TCMALLOC_LIBRARY)
  message(STATUS "Build mini-tcmalloc static library.")

  add_library(${lib_static_target} STATIC ${src_files})

  target_include_directories(
    ${lib_static_target} PUBLIC ${MINI_TCMALLOC_DIR}/include
  )

  install(
    TARGETS ${lib_static_target}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
  set(mini_tcmalloc_lib ${lib_static_target})
endif(USE_STATIC_MINI_TCMALLOC_LIBRARY)

if(USE_SHARED_MINI_TCMALLOC_LIBRARY)
  message(STATUS "Build mini-tcmalloc shared library.")

  add_library(${lib_shared_target} SHARED ${src_files})
  set_target_properties(
    ${lib_shared_target} PROPERTIES VERSION 1.0.0 SOVERSION 1
  )

  target_include_directories(
    ${lib_shared_target} PUBLIC ${MINI_TCMALLOC_DIR}/include
  )

  install(
    TARGETS ${lib_shared_target}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
  set(mini_tcmalloc_lib ${lib_shared_target})
endif(USE_SHARED_MINI_TCMALLOC_LIBRARY)

if(NOT mini_tcmalloc_lib)
  message(FATAL_ERROR "No lib is built!!!")
endif(NOT mini_tcmalloc_lib)

set(mini_tcmalloc_lib
    ${mini_tcmalloc_lib}
    PARENT_SCOPE
)
