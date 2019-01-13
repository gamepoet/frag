#include "utils.h"

static bool is_zero(const void* ptr, int size_bytes) {
  const char* data = (const char*)ptr;
  for (int index = 0; index < size_bytes; ++index) {
    if (data[index] != 0) {
      return false;
    }
  }

  return true;
}

TEST_CASE("frag_alloc_zero", "[general]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  SECTION("it properly zeroes allocations") {
    void* ptr = frag_alloc_zero(system, 128);
    CHECK(is_zero(ptr, 128));
    frag_free(system, ptr);
  }

  SECTION("it handles a zero-sized allocation") {
    void* ptr = frag_alloc_zero(system, 0);
    CHECK(ptr != NULL);
    frag_free(system, ptr);
  }
}

TEST_CASE("frag_alloc_aligned_zero", "[general]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  SECTION("it properly zeroes allocations") {
    void* ptr = frag_alloc_aligned_zero(system, 128, 1024);
    CHECK(is_aligned_ptr(ptr, 1024));
    CHECK(is_zero(ptr, 128));
    frag_free(system, ptr);
  }

  SECTION("it handles a zero-sized allocation") {
    void* ptr = frag_alloc_aligned_zero(system, 0, 1024);
    CHECK(ptr != NULL);
    CHECK(is_aligned_ptr(ptr, 1024));
    frag_free(system, ptr);
  }
}

TEST_CASE("frag_free", "[general]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  SECTION("it accepts a null pointer") {
    frag_free(system, NULL);
  }
}

TEST_CASE("frag_realloc", "[general]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  SECTION("it accepts a null ptr") {
    uint32_t* ptr = (uint32_t*)frag_realloc(system, NULL, 3 * sizeof(uint32_t));
    CHECK(ptr != NULL);
    frag_free(system, ptr);
  }

  SECTION("it frees the memory if new size is zero") {
    uint32_t* ptr = (uint32_t*)frag_alloc(system, 3 * sizeof(uint32_t));
    ptr[0] = 5;
    ptr[1] = 10;
    ptr[2] = 15;
    ptr = (uint32_t*)frag_realloc(system, ptr, 4 * sizeof(uint32_t));
    CHECK(ptr[0] == 5);
    CHECK(ptr[1] == 10);
    CHECK(ptr[2] == 15);
    frag_free(system, ptr);
  }
}
