#include "utils.h"
#include "frag.h"

init_t::init_t() {
  frag_init();
}

init_t::~init_t() {
  frag_shutdown();
}

bool is_aligned(uintptr_t val, size_t alignment) {
  if (0 == (val & (alignment - 1))) {
    return true;
  }
  return false;
}

bool is_aligned_ptr(void* ptr, size_t alignment) {
  return is_aligned((uintptr_t)ptr, alignment);
}
