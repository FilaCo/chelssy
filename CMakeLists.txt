cmake_minimum_required(VERSION 3.28)
project(chelssy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third_party)

# depends on [third_party]
add_subdirectory(src)

# depends on [src]
add_subdirectory(tests)

# depends on [third_party, src]
add_subdirectory(bin)
