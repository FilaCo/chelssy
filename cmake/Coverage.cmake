set(CODE_COVERAGE ${CHELSSY_COVERAGE})
include("${cmake_scripts_SOURCE_DIR}/code-coverage.cmake")

add_code_coverage_all_targets(
    LCOV_EXCLUDE */tests/* */_deps/*
    LLVM_EXCLUDE .*/tests/.* .*/_deps/.*)
