#include "frag.h"
#include "internal.h"
#include <stdio.h>

// the configuration used to initialize this library
static frag_config_t s_config;

static char s_system_allocator_mem[sizeof(frag_allocator_t) + sizeof(system_allocator_impl_t)];
static frag_allocator_t* s_system_allocator;

static void default_assert(const char* file, int line, const char* func, const char* expression, const char* message) {
  fprintf(stderr, "ASSERT FAILURE: %s\n%s\nfile: %s\nline: %d\nfunc: %s\n", expression, message, file, line, func);
  exit(EXIT_FAILURE);
}

void allocator_init(frag_allocator_t* allocator,
                    const char* name,
                    alloc_func_t alloc_func,
                    free_func_t free_func,
                    get_size_func_t get_size_func,
                    shutdown_func_t shutdown_func) {
  allocator->name = name;
  allocator->stats.bytes = 0;
  allocator->stats.count = 0;
  allocator->stats.bytes_peak = 0;
  allocator->stats.count_peak = 0;
  allocator->alloc = alloc_func;
  allocator->free = free_func;
  allocator->get_size = get_size_func;
  allocator->shutdown = shutdown_func;
}

void allocator_shutdown(frag_allocator_t* allocator) {
  frag_assert(allocator->stats.count == 0, "allocator shut down with unfreed allocations");
}

void frag_assert_ex(const char* file, int line, const char* func, const char* expression, const char* message) {
  s_config.assert_handler(file, line, func, expression, message);
}

bool is_pow_2(size_t x) {
  return ((x != 0) && !(x & (x - 1)));
}

uintptr_t align_up(uintptr_t val, size_t alignment) {
  return (val + (alignment - 1)) & ~(alignment - 1);
}

void* align_up_with_offset_ptr(void* cur, size_t alignment, size_t offset) {
  cur = cur + offset;
  cur = (void*)align_up((uintptr_t)cur, alignment);
  return cur;
}

void report_alloc(frag_allocator_t* allocator,
                  void* ptr,
                  size_t size_requested,
                  size_t size_allocated,
                  unsigned int alignment,
                  const char* file,
                  int line,
                  const char* func) {
  ++allocator->stats.count;
  if (allocator->stats.count > allocator->stats.count_peak) {
    allocator->stats.count_peak = allocator->stats.count;
  }

  allocator->stats.bytes += size_allocated;
  if (allocator->stats.bytes > allocator->stats.bytes_peak) {
    allocator->stats.bytes_peak = allocator->stats.bytes;
  }
}

void report_free(frag_allocator_t* allocator, void* ptr, size_t size, const char* file, int line, const char* func) {
  // assert(allocator->stats.count > 0);
  // assert(allocator->stats.bytes >= size);
  --allocator->stats.count;
  allocator->stats.bytes -= size;
}

void report_out_of_memory(frag_allocator_t* allocator,
                          size_t size,
                          unsigned int alignment,
                          const char* file,
                          int line,
                          const char* func) {
  // TODO: explode!?
}

void frag_config_init(frag_config_t* config) {
  if (config != NULL) {
    config->assert_handler = &default_assert;
  }
}

void frag_init(const frag_config_t* config) {
  if (config != NULL) {
    s_config = *config;
  }
  else {
    frag_config_init(&s_config);
  }

  frag_assert(s_system_allocator == NULL, "frag_init is already initialized");

  s_system_allocator = (frag_allocator_t*)s_system_allocator_mem;
  s_system_allocator->impl = (allocator_impl_t*)(s_system_allocator + 1);
  system_init(s_system_allocator);
}

void frag_shutdown() {
  s_system_allocator->shutdown(s_system_allocator);
  s_system_allocator = NULL;
}

void* frag_alloc_ex(frag_allocator_t* allocator,
                    size_t size,
                    size_t alignment,
                    const char* file,
                    int line,
                    const char* func) {
  if (allocator == NULL) {
    return NULL;
  }
  return allocator->alloc(allocator, size, alignment, file, line, func);
}

void frag_free_ex(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  if (allocator == NULL) {
    return;
  }
  return allocator->free(allocator, ptr, file, line, func);
}

frag_allocator_t* frag_system_allocator() {
  return s_system_allocator;
}

void frag_allocator_destroy(frag_allocator_t* owner, frag_allocator_t* allocator) {
  if (owner == NULL || allocator == NULL) {
    return;
  }
  allocator->shutdown(allocator);
  frag_free(owner, allocator);
}

void frag_allocator_stats(const frag_allocator_t* allocator, frag_allocator_stats_t* stats) {
  frag_assert(allocator != NULL, "allocator is null");
  frag_assert(stats != NULL, "stats is null");
  *stats = allocator->stats;
}

frag_allocator_t*
frag_fixed_stack_allocator_create(frag_allocator_t* owner, const char* name, char* buf, size_t buf_size) {
  const size_t alloc_size = sizeof(frag_allocator_t) + sizeof(fixed_stack_allocator_impl_t);
  frag_allocator_t* allocator = (frag_allocator_t*)frag_alloc(owner, alloc_size, 8);
  allocator->impl = (allocator_impl_t*)(allocator + 1);
  fixed_stack_init(allocator, name, buf, buf_size);
  return allocator;
}

frag_allocator_t*
frag_group_allocator_create(frag_allocator_t* owner, const char* name, frag_allocator_t* delegate) {
  const size_t alloc_size = sizeof(frag_allocator_t) + sizeof(group_allocator_impl_t);
  frag_allocator_t* allocator = (frag_allocator_t*)frag_alloc(owner, alloc_size, 8);
  allocator->impl = (allocator_impl_t*)(allocator + 1);
  group_init(allocator, name, delegate);
  return allocator;
}
