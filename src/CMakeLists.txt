add_library(chelssy_core STATIC)

target_include_directories(chelssy_core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(chelssy_core PRIVATE etl::etl)

add_subdirectory(app)
add_subdirectory(chess)
