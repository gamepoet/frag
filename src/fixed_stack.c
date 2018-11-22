#include "internal.h"

typedef struct fixed_stack_allocator_impl_t {
  char* beg;
  char* end;
  char* cur;
} fixed_stack_allocator_impl_t;

typedef struct header_t {
  uint32_t pad;
  uint32_t size;
} header_t;

static size_t fixed_stack_get_size(const frag_allocator_t* allocator, void* ptr) {
  const fixed_stack_allocator_impl_t* impl = (const fixed_stack_allocator_impl_t*)allocator->impl;

  char* alloc_beg = (char*)ptr;
  header_t* header = (header_t*)alloc_beg - 1;
  char* alloc_end = alloc_beg + header->size;
  frag_assert(impl->cur == alloc_end, "tried to free an invalid pointer");

  char* new_cur = alloc_beg - header->pad;
  frag_assert(impl->cur >= impl->beg, "malformed allocation header");
  return (size_t)(alloc_end - new_cur);
}

static void* fixed_stack_alloc(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func, size_t* size_allocated) {
  frag_assert(size <= 0xfffffffful, "requested size exceeds maximum of 2^32.");
  frag_assert(is_pow_2(alignment), "alignment is not a power of 2");
  fixed_stack_allocator_impl_t* impl = (fixed_stack_allocator_impl_t*)allocator->impl;
  char* cur = impl->cur;
  char* end = impl->end;
  char* alloc_beg = (char*)align_up_with_offset_ptr(cur, alignment, sizeof(header_t));
  char* alloc_end = alloc_beg + size;
  if (alloc_end > end) {
    *size_allocated = 0;
    return NULL;
  }

  // just before the alloc, write how much padding was required to get alignment
  header_t* header = (header_t*)alloc_beg - 1;
  header->pad = (uint32_t)(alloc_beg - cur);
  header->size = size;

  impl->cur = alloc_end;

  *size_allocated = (size_t)(alloc_end - cur);
  return alloc_beg;
}

static void fixed_stack_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  if (ptr == NULL) {
    return;
  }
  fixed_stack_allocator_impl_t* impl = (fixed_stack_allocator_impl_t*)allocator->impl;
  header_t* header = (header_t*)ptr - 1;
  impl->cur = (char*)ptr - header->pad;
}

static void fixed_stack_shutdown(frag_allocator_t* allocator) {
}

frag_allocator_t* fixed_stack_create(frag_allocator_t* owner, const char* name, char* buf, size_t size) {
  frag_allocator_desc_t desc = {0};
  desc.name = name;
  desc.alloc = &fixed_stack_alloc;
  desc.free = &fixed_stack_free;
  desc.get_size = &fixed_stack_get_size;
  desc.shutdown = &fixed_stack_shutdown;
  desc.impl_size_bytes = sizeof(fixed_stack_allocator_impl_t);
  frag_allocator_t* allocator = allocator_create(owner, &desc);

  fixed_stack_allocator_impl_t* impl = (fixed_stack_allocator_impl_t*)allocator->impl;
  impl->beg = buf;
  impl->end = buf + size;
  impl->cur = buf;

  return allocator;
}
