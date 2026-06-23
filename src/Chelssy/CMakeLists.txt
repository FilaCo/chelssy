add_library(chelssy_core STATIC)

target_sources(chelssy_core PRIVATE Board.cpp)
target_compile_options(chelssy_core INTERFACE
    -ffreestanding
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
    -nostdinc++
)
target_include_directories(chelssy_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(chelssy_core PUBLIC etl::etl)
