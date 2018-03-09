#include "catch.hpp"
#include "frag.h"

static bool is_aligned(uintptr_t val, size_t alignment) {
  if (0 == (val & (alignment - 1))) {
    return true;
  }
  return false;
}

static bool is_aligned_ptr(void* ptr, size_t alignment) {
  return is_aligned((uintptr_t)ptr, alignment);
}

struct init_t {
  init_t() {
    frag_init();
  }
  ~init_t() {
    frag_shutdown();
  }
};

SCENARIO("can allocate system memory", "[system]") {
  GIVEN("the library is initialized") {
    init_t init;
    frag_allocator_t* system = frag_system_allocator();
    REQUIRE(system != NULL);

    WHEN("memory is allocated") {
      void* ptr = frag_alloc(system, 16, 64);
      REQUIRE(ptr != NULL);

      THEN("it is properly aligned") {
        REQUIRE(is_aligned_ptr(ptr, 64));
      }
      THEN("it can be freed") {
        frag_free(system, ptr);
      }
    }
  }
}
