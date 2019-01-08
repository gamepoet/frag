#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include "frag.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct frag_allocator_debug_t {
  frag_debug_alloc_info_t* allocs;
  unsigned int count;
  unsigned int capacity;
} frag_allocator_debug_t;

typedef struct frag_allocator_t {
  const char* name;
  frag_allocator_stats_t stats;
  frag_allocator_t* owner;
  void* mutex; // NOTE: not std::muteix here to avoid forcing everything to C++ :(
  void* impl;

  void* (*alloc)(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func, size_t* size_allocated);
  void (*free)(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func);
  size_t (*get_size)(const frag_allocator_t* allocator, void* ptr);
  void (*shutdown)(frag_allocator_t* allocator);

  frag_allocator_debug_t debug;
} frag_allocator_t;

frag_allocator_t* allocator_init(void* buffer, size_t buffer_size_bytes, frag_allocator_t* owner, const frag_allocator_desc_t* desc);
void* allocator_alloc(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func, size_t* size_allocated);
void allocator_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func);
size_t allocator_get_size(const frag_allocator_t* allocator, void* ptr);
void allocator_shutdown(frag_allocator_t* allocator);
frag_allocator_t* allocator_create(frag_allocator_t* owner, const frag_allocator_desc_t* desc);

void frag_assert_ex(const char* file, int line, const char* func, const char* expression, const char* message);
#define frag_assert(expr, message) ((expr) ? true : (frag_assert_ex(__FILE__, __LINE__, __func__, #expr, message), false))

bool is_pow_2(size_t x);
void* align_up_with_offset_ptr(void* cur, size_t alignment, size_t offset);

frag_allocator_t* group_create(frag_allocator_t* owner, const char* name, bool needs_lock, frag_allocator_t* delegate);
frag_allocator_t* fixed_stack_create(frag_allocator_t* owner, const char* name, bool needs_lock, char* buf, size_t size);
frag_allocator_t* system_create(void* buffer, size_t buffer_size_bytes, const char* name, bool needs_lock);

#ifdef __cplusplus
}
#endif
