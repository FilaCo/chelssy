add_library(chelssy_core INTERFACE)
target_include_directories(chelssy_core INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(chelssy_core INTERFACE chelssy_options etl::etl)

add_subdirectory(App)
add_subdirectory(Chess)
