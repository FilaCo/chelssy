include(FetchContent)

FetchContent_Declare(
    cmake_scripts
    GIT_REPOSITORY https://github.com/StableCoder/cmake-scripts
    GIT_TAG 25.08
)
FetchContent_MakeAvailable(cmake_scripts)

list(APPEND CMAKE_MODULE_PATH "${cmake_scripts_SOURCE_DIR}")
