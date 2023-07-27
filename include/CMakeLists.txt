option(INSTALL_MINI_TCMALLOC_HEADERS "Install mini-tcmalloc headers." ON)

# set_target_properties(ObjectPool.hpp )

if(INSTALL_MINI_TCMALLOC_HEADERS)
  file(GLOB headers "TCMalloc.h")
  install(
    FILES ${headers}
    DESTINATION include/
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
  )
endif(INSTALL_MINI_TCMALLOC_HEADERS)
