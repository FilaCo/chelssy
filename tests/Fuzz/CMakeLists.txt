function(chelssy_add_fuzz_target name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE chelssy_core)
  target_compile_options(${name} PRIVATE -fsanitize=fuzzer)
  target_link_options(${name} PRIVATE -fsanitize=fuzzer)
endfunction()

add_subdirectory(Board)
add_subdirectory(Fen)
