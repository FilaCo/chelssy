add_executable(chelssy_core_tests)

target_include_directories(chelssy_core_tests PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(chelssy_core_tests PRIVATE
        etl::etl
        GTest::gtest_main
)

gtest_discover_tests(chelssy_core_tests)
