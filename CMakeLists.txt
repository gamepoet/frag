cmake_minimum_required(VERSION 3.8)
project(frag)

set(
  SRCS
  src/fixed_stack.c
  src/frag.c
  src/frag.h
  src/group.c
  src/internal.h
  src/system.c
)

# max out the warning settings for the compilers (why isn't there a generic way to do this?)
if(MSVC)
  string(REPLACE "/W3" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
  string(REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

add_library(frag STATIC ${SRCS})
target_compile_features(frag PRIVATE cxx_std_11)
target_compile_features(frag PUBLIC cxx_variadic_macros)
target_include_directories(
  frag
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:>
)
if(MSVC)
  target_compile_options(frag PRIVATE /W4 /WX /wd4100)
elseif(APPLE)
  target_compile_options(frag PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter)
endif()

set(
  TEST_SRCS
  spec/catch.hpp
  spec/fixed_stack_spec.cpp
  spec/group_spec.cpp
  spec/main.cpp
  spec/system_spec.cpp
  spec/utils.cpp
  spec/utils.h
)

add_executable(test_runner ${TEST_SRCS})
target_compile_features(test_runner PRIVATE cxx_std_11)
target_link_libraries(test_runner frag)
if(MSVC)
  target_compile_options(test_runner PRIVATE /W4 /WX /wd4100)
elseif(APPLE)
  target_compile_options(test_runner PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter)
endif()

enable_testing()
add_test(NAME spec COMMAND test_runner)
