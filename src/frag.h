#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct frag_allocator_t;

// Initializes this library. This will create the system allocator.
void frag_init();

// Tears down this library and frees all allocations.
void frag_shutdown();

// Allocates memory from the given allocator. This is the extended API for when you want full control. Generally you'll
// want to use the frag_alloc() macro.
void* frag_alloc_ex(struct frag_allocator_t* allocator,
                    size_t size,
                    size_t alignment,
                    const char* file,
                    int line,
                    const char* func);

// Frees memory allocated with frag_alloc or frag_realloc. This is the extended API for when you want full control.
// Generally you'll want to use frag_free() instead.
void frag_free_ex(struct frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func);

// Reallocates the given memory, returning the new allocation. This is the extended API for when you want full control.
// Generally you'll want to use the frag_realloc() macro instead.
void* frag_realloc_ex(struct frag_allocator_t* allocator,
                      void* ptr,
                      size_t size,
                      size_t alignment,
                      const char* file,
                      int line,
                      const char* func);

// Allocates memory from the given allocator.
#define frag_alloc(allocator, size, alignment) frag_alloc_ex(allocator, size, alignment, __FILE__, __LINE__, __func__)

// Frees memory from the given allocator.
#define frag_free(allocator, ptr) frag_free_ex(allocator, ptr, __FILE__, __LINE__, __func__)

// Reallocates the given memory.
#define frag_realloc(allocator, ptr, size, alignment)                                                                  \
  frag_realloc_ex(allocator, ptr, size, alignment, __FILE__, __LINE__, __func__)

// Gets the system allocator.
struct frag_allocator_t* frag_system_allocator();

// Destroys the given allocator.
void frag_allocator_destroy(struct frag_allocator_t* owner, struct frag_allocator_t* allocator);

// Creates a stack allocator that works from a fixed buffer
struct frag_allocator_t* frag_fixed_stack_allocator_create(struct frag_allocator_t* owner, const char* name, char* buf, size_t buf_size);

#ifdef __cplusplus
#define FRAG_NEW(allocator, T, ...)                                                                                    \
  (new (frag_alloc_ex(allocator, sizeof(T), alignof(T), __FILE__, __LINE__, __func__)) T(__VA_ARGS__))
#define FRAG_DELETE(allocator, T, ptr)                                                                                 \
  do {                                                                                                                 \
    if (ptr) {                                                                                                         \
      (ptr)->~T();                                                                                                     \
      frag_free(allocator, ptr, __FILE__, __LINE__, __func__);                                                         \
    }                                                                                                                  \
  } while (0)
#endif // __cplusplus

#ifdef __cplusplus
}
#endif
