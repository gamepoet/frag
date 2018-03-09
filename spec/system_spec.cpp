#include "catch.hpp"
#include "frag.h"
#include "utils.h"

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
