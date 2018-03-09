#include "frag.h"
#include "internal.h"

static char s_system_allocator_mem[sizeof(frag_allocator_t) + sizeof(system_allocator_impl_t)];
static frag_allocator_t* s_system_allocator;

void allocator_init(frag_allocator_t* allocator,
                    const char* name,
                    alloc_func_t alloc_func,
                    free_func_t free_func,
                    get_size_func_t get_size_func) {
  allocator->name = name;
  allocator->stat_bytes = 0;
  allocator->stat_count = 0;
  allocator->stat_bytes_peak = 0;
  allocator->stat_count_peak = 0;
  allocator->alloc = alloc_func;
  allocator->free = free_func;
  allocator->get_size = get_size_func;
}

void allocator_shutdown(frag_allocator_t* allocator) {
}

void report_alloc(frag_allocator_t* allocator,
                  void* ptr,
                  size_t size_requested,
                  size_t size_allocated,
                  unsigned int alignment,
                  const char* file,
                  int line,
                  const char* func) {
  ++allocator->stat_count;
  if (allocator->stat_count > allocator->stat_count_peak) {
    allocator->stat_count_peak = allocator->stat_count;
  }

  allocator->stat_bytes += size_allocated;
  if (allocator->stat_bytes > allocator->stat_bytes_peak) {
    allocator->stat_bytes_peak = allocator->stat_bytes;
  }
}

void report_free(frag_allocator_t* allocator, void* ptr, size_t size, const char* file, int line, const char* func) {
  // assert(allocator->stat_count > 0);
  // assert(allocator->stat_bytes >= size);
  --allocator->stat_count;
  allocator->stat_bytes -= size;
}

void report_out_of_memory(frag_allocator_t* allocator,
                          size_t size,
                          unsigned int alignment,
                          const char* file,
                          int line,
                          const char* func) {
  // TODO: explode!?
}

void frag_init() {
  s_system_allocator = (frag_allocator_t*)s_system_allocator_mem;
  s_system_allocator->impl = (struct allocator_impl_t*)(s_system_allocator + 1);
  system_init(s_system_allocator);
}

void frag_shutdown() {
  system_shutdown(s_system_allocator);
}

void* frag_alloc_ex(struct frag_allocator_t* allocator,
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

void frag_free_ex(struct frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  if (allocator == NULL) {
    return;
  }
  return allocator->free(allocator, ptr, file, line, func);
}

struct frag_allocator_t* frag_system_allocator() {
  return s_system_allocator;
}
