cmake_minimum_required(VERSION 3.11)
project(frag LANGUAGES C CXX)

option(FRAG_BUILD_TESTS "Build tests" OFF)

# max out the warning settings for the compilers (why isn't there a generic way to do this?)
if (MSVC)
  string(REPLACE "/W3" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
  string(REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

add_library(
  frag
  STATIC
  src/fixed_stack.c
  src/frag.cpp
  src/frag.h
  src/group.c
  src/internal.h
  src/system.c
)
target_compile_features(
  frag
  PRIVATE
  cxx_std_11
  PUBLIC
  cxx_variadic_macros
)
target_include_directories(
  frag
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:>
)
target_compile_options(
  frag
  PRIVATE
  $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -Wpedantic -Wno-unused-parameter>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /wd4100>
)

# test app
if (FRAG_BUILD_TESTS)
  include(FetchContent)
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v2.5.0
  )
  FetchContent_GetProperties(catch2)
  if (NOT catch2_POPULATED)
    FetchContent_Populate(catch2)
  endif()

  add_executable(
    test_runner
    spec/fixed_stack_spec.cpp
    spec/group_spec.cpp
    spec/main.cpp
    spec/new_delete_spec.cpp
    spec/system_spec.cpp
    spec/utils.cpp
    spec/utils.h
  )
  target_include_directories(test_runner PRIVATE ${catch2_SOURCE_DIR}/single_include/catch2)
  target_compile_features(test_runner PRIVATE cxx_std_11)
  target_link_libraries(test_runner frag)
  target_compile_options(
    test_runner
    PRIVATE
    $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -Wpedantic -Wno-unused-parameter>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /wd4100>
  )

  enable_testing()
  add_test(NAME spec COMMAND test_runner)
endif()
