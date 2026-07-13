cmake_minimum_required(VERSION 3.28)
project(chelssy VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CHELSSY_TESTING "Build the test suite" OFF)
option(CHELSSY_ASAN "Instrument the build with ASan + UBSan (host builds only)" OFF)
option(CHELSSY_FUZZ "Build libFuzzer targets (needs non-Apple Clang)" OFF)
option(CHELSSY_LINT "Run clang-tidy during compilation" OFF)
option(CHELSSY_COVERAGE "Instrument tests and add ccov-* report targets" OFF)

include(StableCoder)
include(Warnings)
include(Sanitizers)
include(Fuzzing)
include(StaticAnalysis)
include(Coverage)
include(Formatting)

add_subdirectory(third_party)

# depends on [third_party]
add_subdirectory(src)

if(CHELSSY_TESTING)
    enable_testing()
    # depends on [src]
    add_subdirectory(tests)
endif()

# depends on [third_party, src]
add_subdirectory(adapters)

# depends on [third_party, src, adapters]
add_subdirectory(entrypoints)
