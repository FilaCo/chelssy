cmake_minimum_required(VERSION 3.28)
project(chelssy VERSION 0.1.0 LANGUAGES CXX)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CHELSSY_TESTING "Build the test suite" OFF)

option(CHELSSY_ASAN "Instrument the build with ASan + UBSan (host builds only)" OFF)
if (CHELSSY_ASAN)
    if (CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "CHELSSY_ASAN is for host builds only")
    endif ()
    # Applied globally (not per-target): the code under test in chelssy_core
    # must be instrumented for ASan to see its stack/global accesses, and
    # mixing instrumented/uninstrumented TUs causes false positives.
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif ()

option(CHELSSY_FUZZ "Build libFuzzer targets (needs non-Apple Clang)" OFF)
if (CHELSSY_FUZZ)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "CHELSSY_FUZZ requires LLVM Clang")
    endif ()
    add_compile_options(-fsanitize=fuzzer-no-link)
endif ()

option(CHELSSY_LINT "Run clang-tidy during compilation" OFF)
if (CHELSSY_LINT)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif ()

add_subdirectory(third_party)

add_library(chelssy_options INTERFACE)
target_compile_features(chelssy_options INTERFACE cxx_std_23)
target_compile_options(chelssy_options INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        >)

# depends on [third_party]
add_subdirectory(src)

if (CHELSSY_TESTING)
    enable_testing()
    # depends on [src]
    add_subdirectory(tests)
endif ()

# depends on [third_party, src]
add_subdirectory(adapters)

# depends on [third_paty, src, adapters]
add_subdirectory(entrypoints)
