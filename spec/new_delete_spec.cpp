#include "utils.h"

struct my_type_t {
  my_type_t() {
    value = 100;
  }

  my_type_t(int32_t in_value) {
    value = in_value;
  }

  int32_t value;
};

TEST_CASE("new_delete", "[cpp]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  SECTION("it calls the default constructor") {
    my_type_t* ptr = frag_new(system, my_type_t);
    CHECK(ptr != NULL);
    CHECK(ptr->value == 100);
    frag_delete(system, ptr);
  }

  SECTION("it calls the correct constructor") {
    my_type_t* ptr = frag_new(system, my_type_t)(25);
    CHECK(ptr != NULL);
    CHECK(ptr->value == 25);
    frag_delete(system, ptr);
  }
}
