#pragma once
#include <stdlib.h>

struct init_t {
  init_t();
  ~init_t();
};

bool is_aligned(uintptr_t val, size_t alignment);
bool is_aligned_ptr(void* ptr, size_t alignment);
