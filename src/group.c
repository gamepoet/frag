#include "internal.h"

typedef struct group_allocator_impl_t {
  frag_allocator_t* delegate;
} group_allocator_impl_t;

static size_t group_get_size(const frag_allocator_t* allocator, void* ptr) {
  group_allocator_impl_t* impl = (group_allocator_impl_t*)allocator->impl;
  return allocator_get_size(impl->delegate, ptr);
}

static void* group_alloc(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func, size_t* size_allocated) {
  group_allocator_impl_t* impl = (group_allocator_impl_t*)allocator->impl;
  return allocator_alloc(impl->delegate, size, alignment, file, line, func, size_allocated);
}

static void group_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  group_allocator_impl_t* impl = (group_allocator_impl_t*)allocator->impl;
  allocator_free(impl->delegate, ptr, file, line, func);
}

static void group_shutdown(frag_allocator_t* allocator) {
}

frag_allocator_t* group_create(frag_allocator_t* owner, const char* name, frag_allocator_t* delegate) {
  frag_allocator_desc_t desc = {0};
  desc.name = name;
  desc.alloc = &group_alloc;
  desc.free = &group_free;
  desc.get_size = &group_get_size;
  desc.shutdown = &group_shutdown;
  desc.impl_size_bytes = sizeof(group_allocator_impl_t);
  frag_allocator_t* allocator = allocator_create(owner, &desc);

  group_allocator_impl_t* impl = (group_allocator_impl_t*)allocator->impl;
  impl->delegate = delegate;

  return allocator;
}
