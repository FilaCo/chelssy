function(chelssy_add_fuzz_target name)
  add_executable(${name} ${ARGN})
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(${name} PRIVATE chelssy_options)
  target_compile_options(${name} PRIVATE -fsanitize=fuzzer)
  target_link_options(${name} PRIVATE -fsanitize=fuzzer)
endfunction()

add_subdirectory(Fen)
