file(GLOB src_files "*.cpp")

message(STATUS "mini_tcmalloc_lib: ${mini_tcmalloc_lib}")

foreach(source ${src_files})
  message(STATUS "Build tutorial file: ${source}")
  get_filename_component(exec_name ${source} NAME_WE)
  add_executable(${exec_name} ${source})
  target_link_libraries(${exec_name} ${mini_tcmalloc_lib})
  target_include_directories(${exec_name} PRIVATE ${MINI_TCMALLOC_DIR}/include)
  install(TARGETS ${exec_name} DESTINATION ${CMAKE_INSTALL_PREFIX}/tutorials)
endforeach()
