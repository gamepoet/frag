#include "internal.h"
#include <malloc/malloc.h>

static void* system_alloc(frag_allocator_t* allocator,
                          size_t size,
                          unsigned int alignment,
                          const char* file,
                          int line,
                          const char* func) {
  void* ptr;
  int status = posix_memalign(&ptr, alignment, size);
  if (status == 0) {
    report_alloc(allocator, ptr, size, malloc_size(ptr), alignment, file, line, func);
    return ptr;
  }
  report_out_of_memory(allocator, size, alignment, file, line, func);
  return NULL;
}

static void system_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  if (ptr != NULL) {
    size_t size = malloc_size(ptr);
    report_free(allocator, ptr, size, file, line, func);
  }
}

static size_t system_get_size(frag_allocator_t* allocator, void* ptr) {
  return malloc_size(ptr);
}

void system_init(frag_allocator_t* allocator) {
  allocator_init(allocator, "system", &system_alloc, &system_free, &system_get_size);
}

void system_shutdown(frag_allocator_t* allocator) {
  allocator_shutdown(allocator);
}
