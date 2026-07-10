add_executable(chelssy_core_tests)

target_include_directories(chelssy_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(chelssy_core_tests PRIVATE
        chelssy_core
        GTest::gtest_main
)

add_subdirectory(Chess)

gtest_discover_tests(chelssy_core_tests NO_PRETTY_VALUES)
