#include "utils.h"

TEST_CASE("group_allocator", "[group]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  frag_allocator_t* group = frag_group_allocator_create(system, "group", true, system);
  DEFER([&] {
    frag_allocator_destroy(system, group);
  });

  SECTION("it allocates properly aligned memory") {
    void* ptr = frag_alloc_aligned(group, 16, 64);
    CHECK(is_aligned_ptr(ptr, 64));
    frag_free(group, ptr);
  }

  SECTION("it tracks memory allocations") {
    frag_allocator_stats_t stats;
    frag_allocator_stats(group, &stats);
    CHECK(stats.count == 0);
    CHECK(stats.count_peak == 0);
    void* ptr1 = frag_alloc_aligned(group, 8, 8);
    frag_allocator_stats(group, &stats);
    CHECK(stats.count == 1);
    CHECK(stats.count_peak == 1);
    void* ptr2 = frag_alloc_aligned(group, 16, 16);
    frag_allocator_stats(group, &stats);
    CHECK(stats.count == 2);
    CHECK(stats.count_peak == 2);
    frag_free(group, ptr2);
    frag_allocator_stats(group, &stats);
    CHECK(stats.count == 1);
    CHECK(stats.count_peak == 2);
    frag_free(group, ptr1);
    frag_allocator_stats(group, &stats);
    CHECK(stats.count == 0);
    CHECK(stats.count_peak == 2);
  }
}

TEST_CASE("group allocator detects memory leaks", "[group]") {
  init_t init(nullptr);
  frag_allocator_t* system = frag_system_allocator();

  frag_allocator_t* group = frag_group_allocator_create(system, "group", true, system);
  DEFER([&] {
    frag_allocator_destroy(system, group);
  });

  SECTION("it detects memory leaks on shutdown") {
    void* ptr = frag_alloc_aligned(group, 16, 32);
    CHECK_THROWS(frag_allocator_destroy(system, group));
    frag_free(group, ptr);
  }
}
