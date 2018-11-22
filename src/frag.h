#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// An allocator instance from which you can manage memory.
typedef struct frag_allocator_t frag_allocator_t;

// This structure is used to describe how to create an allocator. Generally this is only needed if you are writing a
// custom allocator implementation that is not supported by this library.
typedef struct frag_allocator_desc_t {
  // The name of the allocator
  const char* name;

  // Should access to the allocator be protected with a mutex?
  bool needs_lock;

  // The function to call to allocate memory using this allocator.
  void* (*alloc)(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func, size_t* allocated_size);

  // The function to call to free memory using this allocator.
  void (*free)(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func);

  // The function to call to get the allocator size of memory using this allocator.
  size_t (*get_size)(const frag_allocator_t* allocator, void* ptr);

  // The function to call when destroying this allocator.
  void (*shutdown)(frag_allocator_t* allocator);

  // Extra memory to allocate with the allocator for use by the custom implementation.
  size_t impl_size_bytes;
} frag_allocator_desc_t;

typedef struct frag_allocator_stats_t {
  // The number of bytes currently allocated (including overhead)
  size_t bytes;

  // The number of allocations currently active.
  size_t count;

  // The peak number of bytes allocated (including overhead).
  size_t bytes_peak;

  // The peak number of allocations.
  size_t count_peak;
} frag_allocator_stats_t;

typedef void (*frag_assert_handler_t)(const char* file, int line, const char* func, const char* expression, const char* message);

typedef struct frag_config_t {
  // The handler to use for assertion failures.
  frag_assert_handler_t assert_handler;

  // The default alignment to use if no alignment is specified (by giving zero).
  size_t default_alignment;
} frag_config_t;

// Initializes the given config struct to fill it in with the default values.
void frag_config_init(frag_config_t* config);

// Initializes this library. This will create the system allocator.
void frag_init(const frag_config_t* config);

// Tears down this library and frees all allocations.
void frag_shutdown();

// Allocates memory from the given allocator. This is the extended API for when you want full control. Generally you'll
// want to use the frag_alloc() macro.
void* frag_alloc_ex(frag_allocator_t* allocator,
                    size_t size,
                    size_t alignment,
                    const char* file,
                    int line,
                    const char* func);

// Frees memory allocated with frag_alloc. This is the extended API for when you want full control. Generally, you'll
// want to use frag_free() instead.
void frag_free_ex(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func);

// Allocates memory from the given allocator.
#define frag_alloc(allocator, size, alignment) frag_alloc_ex(allocator, size, alignment, __FILE__, __LINE__, __func__)

// Frees memory from the given allocator.
#define frag_free(allocator, ptr) frag_free_ex(allocator, ptr, __FILE__, __LINE__, __func__)

// Gets the system allocator.
frag_allocator_t* frag_system_allocator();

// Allocate an custom allocator not already defined by this library. The memory for the allocator struct itself will be
// allocated from the `owner` allocator.
frag_allocator_t* frag_allocator_create(frag_allocator_t* owner, const frag_allocator_desc_t* desc);

// Destroys the given allocator.
void frag_allocator_destroy(frag_allocator_t* owner, frag_allocator_t* allocator);

// Creates a stack allocator that works from a fixed buffer
frag_allocator_t* frag_fixed_stack_allocator_create(frag_allocator_t* owner, const char* name, bool needs_lock, char* buf, size_t buf_size);

// Creates a group allocator that is just a thin wrapper around another allocator but conceptually groups them together.
frag_allocator_t* frag_group_allocator_create(frag_allocator_t* owner, const char* name, bool needs_lock, frag_allocator_t* delegate);

// Gets the stats for the given allocator.
void frag_allocator_stats(const frag_allocator_t* allocator, frag_allocator_stats_t* stats);

#ifdef __cplusplus
#define FRAG_NEW(allocator, T, ...) (new (frag_alloc_ex(allocator, sizeof(T), alignof(T), __FILE__, __LINE__, __func__)) T(__VA_ARGS__))
#define FRAG_DELETE(allocator, T, ptr)                         \
  do {                                                         \
    if (ptr) {                                                 \
      (ptr)->~T();                                             \
      frag_free(allocator, ptr, __FILE__, __LINE__, __func__); \
    }                                                          \
  } while (0)
#endif // __cplusplus

#ifdef __cplusplus
}
#endif
