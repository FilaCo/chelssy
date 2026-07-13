if(NOT CHELSSY_FUZZ)
    return()
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "CHELSSY_FUZZ requires LLVM Clang")
endif()

add_compile_options(-fsanitize=fuzzer-no-link)
