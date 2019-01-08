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

  // The function to call to get the allocated size of memory using this allocator.
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

typedef struct frag_debug_alloc_info_t {
  void* ptr;
  const char* file;
  const char* func;
  int line;
} frag_debug_alloc_info_t;

typedef struct frag_leak_report_t {
  const frag_debug_alloc_info_t* allocs;
  unsigned int alloc_count;
} frag_leak_report_t;

typedef void (*frag_assert_handler_t)(const char* file, int line, const char* func, const char* expression, const char* message);
typedef void (*frag_report_out_of_memory_handler_t)(const frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func);
typedef void (*frag_report_leak_handler_t)(const frag_allocator_t* allocator, const frag_leak_report_t* report);

typedef struct frag_config_t {
  // The handler to use for assertion failures.
  frag_assert_handler_t assert_handler;

  // The handler to use to report a memory leak.
  frag_report_leak_handler_t report_leak;

  // The handler to use to report memory exhaustion.
  frag_report_out_of_memory_handler_t report_out_of_memory;

  // The default alignment to use if no alignment is specified (by giving zero).
  size_t default_alignment;

  // Enabled more detailed memory leak reporting by tracking the file and line of each outstanding allocation.
  bool enable_detailed_leak_reports;
} frag_config_t;

// Initializes the given config struct to fill it in with the default values.
void frag_config_init(frag_config_t* config);

// Initializes this library. This will create the system allocator.
void frag_lib_init(const frag_config_t* config);

// Tears down this library and frees all allocations.
void frag_lib_shutdown();

// Allocates memory from the given allocator. This is the extended API for when you want full control. Generally you'll
// want to use the frag_alloc() macro.
void* frag_alloc_ex(frag_allocator_t* allocator,
                    size_t size,
                    size_t alignment,
                    const char* file,
                    int line,
                    const char* func);

// Allocates memory from the given allocator that is zeroed. This is the extended API for when you want full control.
// Generally you'll want to use the frag_alloc() macro.
void* frag_alloc_zero_ex(frag_allocator_t* allocator,
                         size_t size,
                         size_t alignment,
                         const char* file,
                         int line,
                         const char* func);

// Frees memory allocated with frag_alloc. This is the extended API for when you want full control. Generally, you'll
// want to use frag_free() instead.
void frag_free_ex(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func);

// Allocates memory with default alignment from the given allocator.
#define frag_alloc(allocator, size) frag_alloc_ex(allocator, size, 0, __FILE__, __LINE__, __func__)

// Allocates aligned memory from the given allocator.
#define frag_alloc_aligned(allocator, size, alignment) frag_alloc_ex(allocator, size, alignment, __FILE__, __LINE__, __func__)

// Allocates memory with default alignment from the given allocator that is zeroed.
#define frag_alloc_zero(allocator, size) frag_alloc_zero_ex(allocator, size, 0, __FILE__, __LINE__, __func__)

// Allocates aligned memory from the given allocator that is zeroed.
#define frag_alloc_aligned_zero(allocator, size, alignment) frag_alloc_zero_ex(allocator, size, alignment, __FILE__, __LINE__, __func__)

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
}
#endif

#ifdef __cplusplus
// Deletes the given object from the given allocator. This is the extended API for when you want full control. Generally
// you'll want to use the frag_new() macro.
template<typename T>
inline void frag_delete_ex(T* ptr, frag_allocator_t* allocator, const char* file, int line, const char* func) {
  ptr->~T();
  frag_free_ex(allocator, ptr, file, line, func);
}

// Allocates and constructs an object of the given type from the given allocator with default alignment.
#define frag_new(allocator, T) new (frag_alloc_ex(allocator, sizeof(T), 0, __FILE__, __LINE__, __func__)) T

// Deletes the given object from the given allocator.
#define frag_delete(allocator, ptr) frag_delete_ex(ptr, allocator, __FILE__, __LINE__, __func__)

#endif // __cplusplus
