if(NOT CHELSSY_LINT)
    return()
endif()

include("${cmake_scripts_SOURCE_DIR}/tools.cmake")
clang_tidy()
