#include "internal.h"

#define GET_IMPL(allocator) ((group_allocator_impl_t*)((allocator)->impl))
#define GET_IMPL_CONST(allocator) ((const group_allocator_impl_t*)((allocator)->impl))

static size_t group_get_size(const frag_allocator_t* allocator, void* ptr) {
  const group_allocator_impl_t* impl = GET_IMPL_CONST(allocator);

  return impl->delegate->get_size(impl->delegate, ptr);
}

static void* group_alloc(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func) {
  group_allocator_impl_t* impl = GET_IMPL(allocator);
  void* ptr = impl->delegate->alloc(impl->delegate, size, alignment, file, line, func);
  if (ptr != NULL) {
    report_alloc(allocator, ptr, size, group_get_size(allocator, ptr), alignment, file, line, func);
  }
  return ptr;
}

static void group_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  if (ptr != NULL) {
    group_allocator_impl_t* impl = GET_IMPL(allocator);
    report_free(allocator, ptr, group_get_size(allocator, ptr), file, line, func);
    impl->delegate->free(impl->delegate, ptr, file, line, func);
  }
}

static void group_shutdown(frag_allocator_t* allocator) {
  allocator_shutdown(allocator);
}

void group_init(frag_allocator_t* allocator, const char* name, frag_allocator_t* delegate) {
  frag_assert(allocator != NULL, "allocator arg is null");
  frag_assert(delegate != NULL, "delegate arg is null");
  allocator_init(allocator, name, &group_alloc, &group_free, &group_get_size, &group_shutdown);
  group_allocator_impl_t* impl = GET_IMPL(allocator);
  impl->delegate = delegate;
}
