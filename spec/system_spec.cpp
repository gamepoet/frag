#include "utils.h"

TEST_CASE("system allocator", "[system]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();
  REQUIRE(system != nullptr);

  SECTION("it can allocate aligned memory") {
    void* ptr = frag_alloc(system, 16, 64);
    CHECK(ptr != nullptr);
    CHECK(is_aligned_ptr(ptr, 64));
    frag_free(system, ptr);
  }
}

TEST_CASE("system allocator detects memory leaks", "[system]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();
  REQUIRE(system != nullptr);

  SECTION("it detects memory leaks on shutdown") {
    void* ptr = frag_alloc(system, 16, 32);
    CHECK_THROWS(frag_shutdown());
    frag_free(system, ptr);
  }
}
