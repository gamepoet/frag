#include "internal.h"

typedef struct header_t {
  uint32_t pad;
  uint32_t size;
} header_t;

#define GET_IMPL(allocator) ((fixed_stack_allocator_impl_t*)((allocator)->impl))
#define GET_IMPL_CONST(allocator) ((const fixed_stack_allocator_impl_t*)((allocator)->impl))

static size_t fixed_stack_get_size(const frag_allocator_t* allocator, void* ptr) {
  const fixed_stack_allocator_impl_t* impl = GET_IMPL_CONST(allocator);

  char* alloc_beg = (char*)ptr;
  header_t* header = (header_t*)alloc_beg - 1;
  char* alloc_end = alloc_beg + header->size;
  frag_assert(impl->cur == alloc_end, "tried to free an invalid pointer");

  char* new_cur = alloc_beg - header->pad;
  frag_assert(impl->cur >= impl->beg, "malformed allocation header");
  return (size_t)(alloc_end - new_cur);
}

static void* fixed_stack_alloc(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func) {
  frag_assert(size <= 0xfffffffful, "requested size exceeds maximum of 2^32.");
  frag_assert(is_pow_2(alignment), "alignment is not a power of 2");
  fixed_stack_allocator_impl_t* impl = GET_IMPL(allocator);
  char* cur = impl->cur;
  char* end = impl->end;
  char* alloc_beg = (char*)align_up_with_offset_ptr(cur, alignment, sizeof(header_t));
  char* alloc_end = alloc_beg + size;
  if (alloc_end > end) {
    report_out_of_memory(allocator, size, alignment, file, line, func);
    return NULL;
  }

  // just before the alloc, write how much padding was required to get alignment
  header_t* header = (header_t*)alloc_beg - 1;
  header->pad = (uint32_t)(alloc_beg - cur);
  header->size = size;

  impl->cur = alloc_end;
  report_alloc(allocator, alloc_beg, size, (size_t)(alloc_end - cur), alignment, file, line, func);
  return alloc_beg;
}

static void fixed_stack_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  if (ptr == NULL) {
    return;
  }
  fixed_stack_allocator_impl_t* impl = GET_IMPL(allocator);

  const size_t size = fixed_stack_get_size(allocator, ptr);
  report_free(allocator, ptr, size, file, line, func);
  header_t* header = (header_t*)ptr - 1;
  impl->cur = (char*)ptr - header->pad;
}

static void fixed_stack_shutdown(frag_allocator_t* allocator) {
  allocator_shutdown(allocator);
}

void fixed_stack_init(frag_allocator_t* allocator, const char* name, char* buf, size_t size) {
  frag_assert(allocator != NULL, "allocator arg is null");
  frag_assert(buf != NULL, "buf arg is null");
  allocator_init(allocator, name, &fixed_stack_alloc, &fixed_stack_free, &fixed_stack_get_size, &fixed_stack_shutdown);
  fixed_stack_allocator_impl_t* impl = GET_IMPL(allocator);
  impl->beg = buf;
  impl->end = buf + size;
  impl->cur = buf;
}
