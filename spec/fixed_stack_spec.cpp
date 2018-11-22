#include "utils.h"

TEST_CASE("fixed_stack allocator", "[fixed_stack]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  const size_t buf_size = 1024;
  char buf[buf_size];
  frag_allocator_t* allocator = frag_fixed_stack_allocator_create(system, "woot", true, buf, buf_size);
  DEFER([&] {
    frag_allocator_destroy(system, allocator);
  });

  SECTION("it can allocate properly aligned memory") {
    void* ptr = frag_alloc(allocator, 16, 64);
    REQUIRE(is_aligned_ptr(ptr, 64));
    frag_free(allocator, ptr);
  }

  SECTION("it allocates memory with sequential addresses") {
    void* ptr1 = frag_alloc(allocator, 16, 16);
    void* ptr2 = frag_alloc(allocator, 32, 8);
    void* ptr3 = frag_alloc(allocator, 23, 128);
    REQUIRE(is_aligned_ptr(ptr1, 16));
    REQUIRE(is_aligned_ptr(ptr2, 8));
    REQUIRE(is_aligned_ptr(ptr3, 128));

    REQUIRE((uintptr_t)ptr2 > (uintptr_t)ptr1);
    REQUIRE((uintptr_t)ptr3 > (uintptr_t)ptr2);

    frag_free(allocator, ptr3);
    frag_free(allocator, ptr2);
    frag_free(allocator, ptr1);
  }
}

TEST_CASE("fixed_stack allocator detects memory leaks", "[fixed_stack]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  const size_t buf_size = 1024;
  char buf[buf_size];
  frag_allocator_t* allocator = frag_fixed_stack_allocator_create(system, "woot", true, buf, buf_size);
  DEFER([&] {
    frag_allocator_destroy(system, allocator);
  });

  SECTION("it detects memory leaks on shutdown") {
    void* ptr = frag_alloc(allocator, 16, 32);
    CHECK_THROWS(frag_allocator_destroy(system, allocator));
    frag_free(allocator, ptr);
  }
}

TEST_CASE("fixed_stack allocator requires ordered frees", "[fixed_stack]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  const size_t buf_size = 1024;
  char buf[buf_size];
  frag_allocator_t* allocator = frag_fixed_stack_allocator_create(system, "woot", true, buf, buf_size);
  DEFER([&] {
    frag_allocator_destroy(system, allocator);
  });

  SECTION("it asserts when freeing a pointer that's not on top") {
    void* ptr1 = frag_alloc(allocator, 8, 8);
    void* ptr2 = frag_alloc(allocator, 16, 16);
    CHECK_THROWS(frag_free(allocator, ptr1));
    frag_free(allocator, ptr2);
    frag_free(allocator, ptr1);
  }
}
