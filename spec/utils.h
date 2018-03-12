#pragma once
#include "catch.hpp"
#include "frag.h"
#include <functional>
#include <stdlib.h>

struct init_t {
  init_t(struct frag_config_t* config);
  ~init_t();
};

bool is_aligned(uintptr_t val, size_t alignment);
bool is_aligned_ptr(void* ptr, size_t alignment);

struct defer_t {
  defer_t(std::function<void()> func);
  ~defer_t();

  std::function<void()> deferred_func;
};

#define TOKEN_PASTE_IMPL(a, b) a ## b
#define TOKEN_PASTE(a, b) TOKEN_PASTE_IMPL(a, b)
#define DEFER(func) defer_t TOKEN_PASTE(deferred_, __LINE__)(func)
