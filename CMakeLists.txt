cmake_minimum_required(VERSION 3.28)
project(chelssy VERSION 0.1.0 LANGUAGES CXX)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build the test suite" OFF)

add_subdirectory(third_party)

# depends on [third_party]
add_subdirectory(src)

if (BUILD_TESTING)
    enable_testing()
    # depends on [src]
    add_subdirectory(tests)
endif ()

# depends on [third_party, src]
add_subdirectory(adapters)

# depends on [third_paty, src, adapters]
add_subdirectory(entrypoints)
