#pragma once
#include "frag.h"
#include <stdbool.h>
#include <stdlib.h>

struct allocator_impl_t;
struct frag_allocator_t;

typedef void* (*alloc_func_t)(struct frag_allocator_t* allocator,
                              size_t size,
                              size_t alignment,
                              const char* file,
                              int line,
                              const char* func);
typedef void (
    *free_func_t)(struct frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func);
typedef size_t (*get_size_func_t)(const struct frag_allocator_t* allocator, void* ptr);
typedef void (*shutdown_func_t)(struct frag_allocator_t* allocator);

typedef struct frag_allocator_t {
  const char* name;
  struct frag_allocator_stats_t stats;
  struct allocator_impl_t* impl;

  alloc_func_t alloc;
  free_func_t free;
  get_size_func_t get_size;
  shutdown_func_t shutdown;
} frag_allocator_t;

void allocator_init(frag_allocator_t* allocator,
                    const char* name,
                    alloc_func_t alloc_func,
                    free_func_t free_func,
                    get_size_func_t get_size_func,
                    shutdown_func_t shutdown_func);
void allocator_shutdown(frag_allocator_t* allocator);

void frag_assert_ex(const char* file, int line, const char* func, const char* expression, const char* message);
#define frag_assert(expr, message)                                                                                     \
  ((expr) ? true : (frag_assert_ex(__FILE__, __LINE__, __func__, #expr, message), false))

bool is_pow_2(size_t x);
void* align_up_with_offset_ptr(void* cur, size_t alignment, size_t offset);

void report_alloc(frag_allocator_t* allocator,
                  void* ptr,
                  size_t size_requested,
                  size_t size_allocated,
                  unsigned int alignment,
                  const char* file,
                  int line,
                  const char* func);
void report_free(frag_allocator_t* allocator, void* ptr, size_t size, const char* file, int line, const char* func);
void report_out_of_memory(frag_allocator_t* allocator,
                          size_t size,
                          unsigned int alignment,
                          const char* file,
                          int line,
                          const char* func);

typedef struct system_allocator_impl_t {
} system_allocator_impl_t;

void system_init(frag_allocator_t* allocator);

typedef struct fixed_stack_allocator_impl_t {
  char* beg;
  char* end;
  char* cur;
} fixed_stack_allocator_impl_t;

void fixed_stack_init(frag_allocator_t* allocator, const char* name, char* buf, size_t size);

typedef struct group_allocator_impl_t {
  struct frag_allocator_t* delegate;
} group_allocator_impl_t;

void group_init(frag_allocator_t* allocator, const char* name, frag_allocator_t* delegate);
