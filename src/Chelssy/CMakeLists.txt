add_library(chelssy_core STATIC)

target_sources(chelssy_core
    PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES Chess.cppm
)

target_compile_options(chelssy_core PRIVATE
    -ffreestanding
    -fno-exceptions
    -fno-rtti
    -fno-threadsafe-statics
    -nostdinc++
)
 
target_link_libraries(chelssy_core PUBLIC etl::etl)
