cmake_minimum_required(VERSION 3.24)
project(chelssy VERSION 0.1.0 LANGUAGES CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()

add_subdirectory(third_party)

# depends on [third_party]
add_subdirectory(src)

# depends on [src]
add_subdirectory(tests)

# depends on [third_party, src]
add_subdirectory(bin)
