include(GoogleTest)

add_subdirectory(Core)

if (CHELSSY_FUZZ)
  add_subdirectory(Fuzz)
endif ()
