if(NOT CHELSSY_ASAN)
    return()
endif()

if(CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR "CHELSSY_ASAN is for host builds only")
endif()

include("${cmake_scripts_SOURCE_DIR}/sanitizers.cmake")

# Applied globally (not per-target): the code under test in chelssy_core
# must be instrumented for ASan to see its stack/global accesses, and
# mixing instrumented/uninstrumented TUs causes false positives.
set_sanitizer_options(address DEFAULT
    -fno-sanitize-recover=all -fno-omit-frame-pointer)
set_sanitizer_options(undefined DEFAULT -fno-sanitize-recover=all)
add_sanitizer_support(address undefined)
