cmake_minimum_required(VERSION 3.8)
project(frag)

set(
  SRCS
  src/frag.c
  src/frag.h
  src/internal.h
  src/system.c
)

# TODO: max out the warning settings for the compilers (why isn't there a generic way to do this?)
#if(MSVC)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
#endif()

add_library(frag STATIC ${SRCS})
target_compile_features(frag PRIVATE cxx_std_11)
target_compile_features(frag PUBLIC cxx_variadic_macros)
target_include_directories(
  frag
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:>
)

set(
  TEST_SRCS
  spec/catch.hpp
  spec/main.cpp
  spec/system_spec.cpp
)

add_executable(test_frag ${TEST_SRCS})
target_compile_features(test_frag PRIVATE cxx_std_11)
target_link_libraries(test_frag frag)

enable_testing()
add_test(NAME spec COMMAND test_frag)
