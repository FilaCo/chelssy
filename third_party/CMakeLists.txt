include(FetchContent)

FetchContent_Declare(
    etl
    GIT_REPOSITORY https://github.com/ETLCPP/etl
    GIT_TAG 20.47.1
)

set(NO_STL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(etl)
target_compile_definitions(etl INTERFACE ETL_NO_STL)
