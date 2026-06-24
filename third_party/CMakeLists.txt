include(FetchContent)

FetchContent_Declare(
    etl
    GIT_REPOSITORY https://github.com/ETLCPP/etl
    GIT_TAG 20.47.1
)
set(NO_STL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(etl)
if(NOT BUILD_TESTING)
    target_compile_definitions(etl INTERFACE ETL_NO_STL)
endif()

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.17.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()
