set(CODE_COVERAGE ${CHELSSY_COVERAGE})
include("${cmake_scripts_SOURCE_DIR}/code-coverage.cmake")

if(CODE_COVERAGE)
    # llvm-cov has no LCOV_EXCL_* marker support, so assert() lines would
    # count as permanently-missed branches; compile them out here. The
    # `test` preset still runs the suite with asserts armed.
    add_compile_definitions(NDEBUG)
endif()

add_code_coverage_all_targets(
    LCOV_EXCLUDE */tests/* */_deps/*
    LLVM_EXCLUDE .*/tests/.* .*/_deps/.*)
