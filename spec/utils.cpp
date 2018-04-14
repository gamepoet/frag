#include "utils.h"
#include "frag.h"

static void test_assert_handler(const char* file, int line, const char* func, const char* expression, const char* message) {
  throw std::runtime_error(message);
}

init_t::init_t(struct frag_config_t* config) {
  frag_config_t config_default;
  if (!config) {
    frag_config_init(&config_default);
    config_default.assert_handler = &test_assert_handler;
    config = &config_default;
  }
  frag_init(config);
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

defer_t::defer_t(std::function<void()> func)
    : deferred_func(func) {
}

defer_t::~defer_t() {
  deferred_func();
}
