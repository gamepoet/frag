#include <mutex>
#include <stdio.h>
#include <string.h>
#include "frag.h"
#include "internal.h"

class optional_lock_guard_t {
public:
  optional_lock_guard_t(std::mutex* mutex) {
    m_mutex = mutex;
    if (m_mutex != NULL) {
      m_mutex->lock();
    }
  }
  ~optional_lock_guard_t() {
    if (m_mutex != NULL) {
      m_mutex->unlock();
    }
  }

private:
  std::mutex* m_mutex;
};

// the configuration used to initialize this library
static frag_config_t s_config;

#define SYSTEM_ALLOCATOR_MEM_SIZE_BYTES (sizeof(frag_allocator_t) + sizeof(std::mutex) + (7 * sizeof(char)))
static char s_system_allocator_mem[SYSTEM_ALLOCATOR_MEM_SIZE_BYTES];
static frag_allocator_t* s_system_allocator;

static void default_assert(const char* file, int line, const char* func, const char* expression, const char* message) {
  fprintf(stderr, "ASSERT FAILURE: %s\n%s\nfile: %s\nline: %d\nfunc: %s\n", expression, message, file, line, func);
  exit(EXIT_FAILURE);
}

static void default_report_leak(const frag_allocator_t* allocator) {
  char message[128];
  snprintf(message, 128, "leak detected. allocator=%s, count=%zu, size=%zu", allocator->name, allocator->stats.count, allocator->stats.bytes);
  message[127] = 0;
  frag_assert(false, message);
}

static void default_report_out_of_memory(const frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func) {
  frag_assert(false, "out of memory");
}

void frag_assert_ex(const char* file, int line, const char* func, const char* expression, const char* message) {
  s_config.assert_handler(file, line, func, expression, message);
}

bool is_pow_2(size_t x) {
  return ((x != 0) && !(x & (x - 1)));
}

uintptr_t align_up(uintptr_t val, size_t alignment) {
  return (val + (alignment - 1)) & ~(alignment - 1);
}

void* align_up_with_offset_ptr(void* cur, size_t alignment, size_t offset) {
  cur = (void*)((char*)cur + offset);
  cur = (void*)align_up((uintptr_t)cur, alignment);
  return cur;
}

static void report_alloc(frag_allocator_t* allocator,
                         void* ptr,
                         size_t size_requested,
                         size_t size_allocated,
                         size_t alignment,
                         const char* file,
                         int line,
                         const char* func) {
  ++allocator->stats.count;
  if (allocator->stats.count > allocator->stats.count_peak) {
    allocator->stats.count_peak = allocator->stats.count;
  }

  allocator->stats.bytes += size_allocated;
  if (allocator->stats.bytes > allocator->stats.bytes_peak) {
    allocator->stats.bytes_peak = allocator->stats.bytes;
  }
}

static void report_free(frag_allocator_t* allocator, void* ptr, size_t size, const char* file, int line, const char* func) {
  // assert(allocator->stats.count > 0);
  // assert(allocator->stats.bytes >= size);
  --allocator->stats.count;
  allocator->stats.bytes -= size;
}

static void report_leak(const frag_allocator_t* allocator) {
  s_config.report_leak(allocator);
}

static void report_out_of_memory(const frag_allocator_t* allocator,
                                 size_t size,
                                 size_t alignment,
                                 const char* file,
                                 int line,
                                 const char* func) {
  s_config.report_out_of_memory(allocator, size, alignment, file, line, func);
}

static size_t calc_allocator_size(const frag_allocator_desc_t* desc) {
  size_t size = sizeof(frag_allocator_t);
  if (desc->needs_lock) {
    size += sizeof(std::mutex);
  }
  size += desc->impl_size_bytes;
  size += (strlen(desc->name) + 1) * sizeof(char);
  return size;
}

frag_allocator_t* allocator_init(void* buffer, size_t buffer_size_bytes, frag_allocator_t* owner, const frag_allocator_desc_t* desc) {
  // verify the size matches
  frag_assert(buffer_size_bytes == calc_allocator_size(desc), "allocator buffer size mismatch");

  // build the structure layout from the buffer memory
  char* cursor = (char*)buffer;
  frag_allocator_t* allocator = (frag_allocator_t*)cursor;
  cursor += sizeof(frag_allocator_t);
  void* impl = NULL;
  std::mutex* mutex = NULL;
  if (desc->needs_lock) {
    mutex = new (cursor) std::mutex();
    cursor += sizeof(std::mutex);
  }
  if (desc->impl_size_bytes > 0) {
    impl = cursor;
    cursor += desc->impl_size_bytes;
  }
  char* name = (char*)cursor;

  memmove(name, desc->name, strlen(desc->name) + 1);

  allocator->name = name;
  allocator->stats.bytes = 0;
  allocator->stats.count = 0;
  allocator->stats.bytes_peak = 0;
  allocator->stats.count_peak = 0;
  allocator->owner = owner;
  allocator->mutex = mutex;
  allocator->impl = impl;
  allocator->alloc = desc->alloc;
  allocator->free = desc->free;
  allocator->get_size = desc->get_size;
  allocator->shutdown = desc->shutdown;

  return allocator;
}

void allocator_shutdown(frag_allocator_t* allocator) {
  if (allocator->stats.count != 0) {
    report_leak(allocator);
  }
  allocator->shutdown(allocator);
  std::mutex* mutex = (std::mutex*)allocator->mutex;
  if (mutex != NULL) {
    mutex->~mutex();
  }
}

void* allocator_alloc(frag_allocator_t* allocator, size_t size, size_t alignment, const char* file, int line, const char* func, size_t* size_allocated) {
  if (alignment == 0) {
    alignment = s_config.default_alignment;
  }

  // protect access to this allocator if necessary
  optional_lock_guard_t lock((std::mutex*)allocator->mutex);

  void* ptr = allocator->alloc(allocator, size, alignment, file, line, func, size_allocated);
  if (ptr != NULL) {
    report_alloc(allocator, ptr, size, *size_allocated, alignment, file, line, func);
  }
  else {
    *size_allocated = 0;
    report_out_of_memory(allocator, size, alignment, file, line, func);
  }

  return ptr;
}

void allocator_free(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  // protect access to this allocator if necessary
  optional_lock_guard_t lock((std::mutex*)allocator->mutex);

  size_t size_allocated = allocator->get_size(allocator, ptr);
  allocator->free(allocator, ptr, file, line, func);
  report_free(allocator, ptr, size_allocated, file, line, func);
}

size_t allocator_get_size(const frag_allocator_t* allocator, void* ptr) {
  // protect access to this allocator if necessary
  optional_lock_guard_t lock((std::mutex*)allocator->mutex);

  return allocator->get_size(allocator, ptr);
}

frag_allocator_t* allocator_create(frag_allocator_t* owner, const frag_allocator_desc_t* desc) {
  const size_t buffer_size_bytes = calc_allocator_size(desc);
  size_t size_allocated;
  void* buffer = allocator_alloc(owner, buffer_size_bytes, 0, __FILE__, __LINE__, __func__, &size_allocated);
  return allocator_init(buffer, buffer_size_bytes, owner, desc);
}

void frag_config_init(frag_config_t* config) {
  if (config != NULL) {
    config->assert_handler = &default_assert;
    config->report_leak = &default_report_leak;
    config->report_out_of_memory = &default_report_out_of_memory;
    config->default_alignment = 16;
  }
}

void frag_lib_init(const frag_config_t* config) {
  if (config != NULL) {
    s_config = *config;
  }
  else {
    frag_config_init(&s_config);
  }

  frag_assert(s_system_allocator == NULL, "frag_init is already initialized");

  s_system_allocator = system_create(s_system_allocator_mem, SYSTEM_ALLOCATOR_MEM_SIZE_BYTES, "system", true);
}

void frag_lib_shutdown() {
  allocator_shutdown(s_system_allocator);
  s_system_allocator = NULL;
}

void* frag_alloc_ex(frag_allocator_t* allocator,
                    size_t size,
                    size_t alignment,
                    const char* file,
                    int line,
                    const char* func) {
  if (allocator == NULL) {
    return NULL;
  }
  size_t size_allocated;
  return allocator_alloc(allocator, size, alignment, file, line, func, &size_allocated);
}

void frag_free_ex(frag_allocator_t* allocator, void* ptr, const char* file, int line, const char* func) {
  if (allocator == NULL) {
    return;
  }
  allocator_free(allocator, ptr, file, line, func);
}

frag_allocator_t* frag_system_allocator() {
  return s_system_allocator;
}

void frag_allocator_destroy(frag_allocator_t* owner, frag_allocator_t* allocator) {
  if (owner == NULL || allocator == NULL) {
    return;
  }
  allocator_shutdown(allocator);
  allocator_free(owner, allocator, __FILE__, __LINE__, __func__);
}

void frag_allocator_stats(const frag_allocator_t* allocator, frag_allocator_stats_t* stats) {
  frag_assert(allocator != NULL, "allocator is null");
  frag_assert(stats != NULL, "stats is null");

  // protect access to this allocator if necessary
  optional_lock_guard_t lock((std::mutex*)allocator->mutex);

  *stats = allocator->stats;
}

frag_allocator_t* frag_fixed_stack_allocator_create(frag_allocator_t* owner, const char* name, bool needs_lock, char* buf, size_t buf_size) {
  return fixed_stack_create(owner, name, needs_lock, buf, buf_size);
}

frag_allocator_t* frag_group_allocator_create(frag_allocator_t* owner, const char* name, bool needs_lock, frag_allocator_t* delegate) {
  return group_create(owner, name, needs_lock, delegate);
}
