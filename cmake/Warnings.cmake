add_library(chelssy_options INTERFACE)
target_compile_features(chelssy_options INTERFACE cxx_std_23)
target_compile_options(chelssy_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    >)
