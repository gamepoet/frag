#include "catch.hpp"
#include "frag.h"
#include "utils.h"

SCENARIO("can alloc memory", "[fixed_stack]") {
  GIVEN("a stack is created") {
    init_t init;
    frag_allocator_t* system = frag_system_allocator();

    const size_t buf_size = 1024;
    char buf[buf_size];
    frag_allocator_t* allocator = frag_fixed_stack_allocator_create(system, "woot", buf, buf_size);

    WHEN("memory is allocated") {
      void* ptr = frag_alloc(allocator, 16, 64);
      THEN("it is properly aligned") {
        REQUIRE(is_aligned_ptr(ptr, 64));
      }
      THEN("it can be freed") {
        frag_free(allocator, ptr);
      }

      WHEN("more memory is allocated") {
        void* ptr2 = frag_alloc(allocator, 32, 8);
        void* ptr3 = frag_alloc(allocator, 23, 128);
        THEN("they are properly aligned") {
          REQUIRE(is_aligned_ptr(ptr2, 8));
          REQUIRE(is_aligned_ptr(ptr3, 128));
        }
        THEN("the allocations are in order in memory") {
          REQUIRE((uintptr_t)ptr3 > (uintptr_t)ptr2);
        }
        THEN("they can be freed") {
          frag_free(allocator, ptr3);
          frag_free(allocator, ptr2);
        }
      }
    }

    frag_allocator_destroy(system, allocator);
  }
}
