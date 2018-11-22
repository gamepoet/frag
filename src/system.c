#include <malloc/malloc.h>
#include "internal.h"

static void* system_alloc(frag_allocator_t* allocator,
                          size_t size,
                          size_t alignment,
                          const char* file,
                          int line,
                          const char* func,
                          size_t* size_allocated) {
  void* ptr = NULL;
  int status = posix_memalign(&ptr, alignment, size);
  if (status == 0) {
    *size_allocated = malloc_size(ptr);
  }
  else {
    *size_allocated = 0;
  }
  return ptr;
}

static void system_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  free(ptr);
}

static size_t system_get_size(const frag_allocator_t* allocator, void* ptr) {
  return malloc_size(ptr);
}

static void system_shutdown(frag_allocator_t* allocator) {
}

frag_allocator_t* system_create(void* buffer, size_t buffer_size_bytes, const char* name) {
  frag_allocator_desc_t desc = {0};
  desc.name = name;
  desc.alloc = &system_alloc;
  desc.free = &system_free;
  desc.get_size = &system_get_size;
  desc.shutdown = &system_shutdown;
  desc.impl_size_bytes = 0;
  frag_allocator_t* allocator = allocator_init(buffer, buffer_size_bytes, NULL, &desc);

  return allocator;
}
