include("${cmake_scripts_SOURCE_DIR}/formatting.cmake")

file(GLOB_RECURSE CHELSSY_FORMAT_CXX_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/*.h"
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    "${PROJECT_SOURCE_DIR}/adapters/*.h"
    "${PROJECT_SOURCE_DIR}/adapters/*.cpp"
    "${PROJECT_SOURCE_DIR}/entrypoints/*.h"
    "${PROJECT_SOURCE_DIR}/entrypoints/*.cpp")
clang_format(format-cxx ${CHELSSY_FORMAT_CXX_SOURCES})

file(GLOB_RECURSE CHELSSY_FORMAT_CMAKE_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/cmake/*.cmake"
    "${PROJECT_SOURCE_DIR}/src/*CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/tests/*CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/adapters/*CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/entrypoints/*CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/third_party/*CMakeLists.txt")
cmake_format(format-cmake
    "${PROJECT_SOURCE_DIR}/CMakeLists.txt"
    ${CHELSSY_FORMAT_CMAKE_SOURCES})

# clang_format() only creates the umbrella when clang-format exists; keep
# the fmt workflow runnable (with a hint) instead of failing on a missing
# ninja target.
if(NOT TARGET format)
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo
        "clang-format not found - nothing formatted")
endif()
if(TARGET format-cmake)
    add_dependencies(format format-cmake)
endif()
