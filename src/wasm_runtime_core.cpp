#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <new>

extern "C" {

extern unsigned char __heap_base;

static uintptr_t heap_cursor = 0;
int errno = 0;
FILE* stdin = nullptr;
FILE* stdout = nullptr;
FILE* stderr = nullptr;

void abort() {
  __builtin_trap();
}

void exit(int) {
  abort();
}

void _Exit(int) {
  abort();
}

void* memset(void* dest, int value, size_t count) {
  unsigned char* out = static_cast<unsigned char*>(dest);
  for (size_t i = 0; i < count; ++i) {
    out[i] = static_cast<unsigned char>(value);
  }
  return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
  unsigned char* out = static_cast<unsigned char*>(dest);
  const unsigned char* in = static_cast<const unsigned char*>(src);
  for (size_t i = 0; i < count; ++i) {
    out[i] = in[i];
  }
  return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
  unsigned char* out = static_cast<unsigned char*>(dest);
  const unsigned char* in = static_cast<const unsigned char*>(src);
  if (out < in) {
    for (size_t i = 0; i < count; ++i) {
      out[i] = in[i];
    }
  } else if (out > in) {
    for (size_t i = count; i > 0; --i) {
      out[i - 1] = in[i - 1];
    }
  }
  return dest;
}

int memcmp(const void* left, const void* right, size_t count) {
  const unsigned char* a = static_cast<const unsigned char*>(left);
  const unsigned char* b = static_cast<const unsigned char*>(right);
  for (size_t i = 0; i < count; ++i) {
    if (a[i] != b[i]) {
      return static_cast<int>(a[i]) - static_cast<int>(b[i]);
    }
  }
  return 0;
}

void* memchr(const void* ptr, int value, size_t count) {
  const unsigned char* in = static_cast<const unsigned char*>(ptr);
  for (size_t i = 0; i < count; ++i) {
    if (in[i] == static_cast<unsigned char>(value)) {
      return const_cast<unsigned char*>(in + i);
    }
  }
  return nullptr;
}

size_t strlen(const char* str) {
  size_t len = 0;
  while (str[len] != '\0') {
    ++len;
  }
  return len;
}

size_t strnlen(const char* str, size_t max_len) {
  size_t len = 0;
  while (len < max_len && str[len] != '\0') {
    ++len;
  }
  return len;
}

char* strchr(const char* str, int value) {
  for (size_t i = 0;; ++i) {
    if (str[i] == static_cast<char>(value)) {
      return const_cast<char*>(str + i);
    }
    if (str[i] == '\0') {
      return nullptr;
    }
  }
}

int strcmp(const char* left, const char* right) {
  size_t i = 0;
  while (left[i] != '\0' && left[i] == right[i]) {
    ++i;
  }
  return static_cast<unsigned char>(left[i]) - static_cast<unsigned char>(right[i]);
}

int strncmp(const char* left, const char* right, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    if (left[i] != right[i] || left[i] == '\0') {
      return static_cast<unsigned char>(left[i]) - static_cast<unsigned char>(right[i]);
    }
  }
  return 0;
}

char* strncpy(char* dest, const char* src, size_t count) {
  size_t i = 0;
  for (; i < count && src[i] != '\0'; ++i) {
    dest[i] = src[i];
  }
  for (; i < count; ++i) {
    dest[i] = '\0';
  }
  return dest;
}

char* strerror(int) {
  return const_cast<char*>("unsupported");
}

void* malloc(size_t size) {
  if (heap_cursor == 0) {
    heap_cursor = reinterpret_cast<uintptr_t>(&__heap_base);
  }
  size = (size + 7u) & ~static_cast<size_t>(7u);
  void* ptr = reinterpret_cast<void*>(heap_cursor);
  heap_cursor += size;
  return ptr;
}

void* calloc(size_t count, size_t size) {
  size_t total = count * size;
  void* ptr = malloc(total);
  if (ptr != nullptr) {
    memset(ptr, 0, total);
  }
  return ptr;
}

void qsort(void*, size_t, size_t, int (*)(const void*, const void*)) {}

char* getenv(const char*) {
  return nullptr;
}

char* realpath(const char*, char*) {
  errno = ENOENT;
  return nullptr;
}

void free(void*) {}

void* realloc(void* ptr, size_t size) {
  void* next = malloc(size);
  if (ptr != nullptr && next != nullptr) {
    memcpy(next, ptr, size);
  }
  return next;
}

char* strdup(const char* str) {
  size_t size = strlen(str) + 1;
  char* out = static_cast<char*>(malloc(size));
  if (out != nullptr) {
    memcpy(out, str, size);
  }
  return out;
}

int abs(int value) {
  return value < 0 ? -value : value;
}

static unsigned int rand_state = 1;

void srand(unsigned int seed) {
  rand_state = seed == 0 ? 1 : seed;
}

int rand() {
  rand_state = rand_state * 1103515245u + 12345u;
  return static_cast<int>((rand_state >> 16) & 0x7fffu);
}

long strtol(const char* str, char** end, int) {
  if (end != nullptr) {
    *end = const_cast<char*>(str);
  }
  return 0;
}

unsigned long strtoul(const char* str, char** end, int) {
  if (end != nullptr) {
    *end = const_cast<char*>(str);
  }
  return 0;
}

long long strtoll(const char* str, char** end, int) {
  if (end != nullptr) {
    *end = const_cast<char*>(str);
  }
  return 0;
}

unsigned long long strtoull(const char* str, char** end, int) {
  if (end != nullptr) {
    *end = const_cast<char*>(str);
  }
  return 0;
}

float strtof(const char* str, char** end) {
  if (end != nullptr) {
    *end = const_cast<char*>(str);
  }
  return 0;
}

double strtod(const char* str, char** end) {
  if (end != nullptr) {
    *end = const_cast<char*>(str);
  }
  return 0;
}

int isalnum(int value) {
  return isalpha(value) || isdigit(value);
}

int isalpha(int value) {
  return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z');
}

int isdigit(int value) {
  return value >= '0' && value <= '9';
}

int islower(int value) {
  return value >= 'a' && value <= 'z';
}

int isspace(int value) {
  return value == ' ' || value == '\t' || value == '\n' || value == '\r' || value == '\f' || value == '\v';
}

int isupper(int value) {
  return value >= 'A' && value <= 'Z';
}

int isxdigit(int value) {
  return isdigit(value) || (value >= 'A' && value <= 'F') || (value >= 'a' && value <= 'f');
}

int tolower(int value) {
  return isupper(value) ? value + ('a' - 'A') : value;
}

int toupper(int value) {
  return islower(value) ? value - ('a' - 'A') : value;
}

int snprintf(char*, unsigned long, const char*, ...) {
  return -1;
}

int vsnprintf(char*, unsigned long, const char*, __builtin_va_list) {
  return -1;
}

int printf(const char*, ...) {
  return -1;
}

int fprintf(FILE*, const char*, ...) {
  return -1;
}

static void umul64wide(uint64_t left, uint64_t right, uint64_t* high, uint64_t* low) {
  const uint64_t left_low = static_cast<uint32_t>(left);
  const uint64_t left_high = left >> 32;
  const uint64_t right_low = static_cast<uint32_t>(right);
  const uint64_t right_high = right >> 32;

  const uint64_t product_0 = left_low * right_low;
  const uint64_t product_1 = left_low * right_high;
  const uint64_t product_2 = left_high * right_low;
  const uint64_t product_3 = left_high * right_high;
  const uint64_t middle = (product_0 >> 32) + static_cast<uint32_t>(product_1) +
                          static_cast<uint32_t>(product_2);

  *low = (middle << 32) | static_cast<uint32_t>(product_0);
  *high = product_3 + (product_1 >> 32) + (product_2 >> 32) + (middle >> 32);
}

__int128 __multi3(__int128 left, __int128 right) {
  const auto unsigned_left = static_cast<unsigned __int128>(left);
  const auto unsigned_right = static_cast<unsigned __int128>(right);
  const auto left_low = static_cast<uint64_t>(unsigned_left);
  const auto left_high = static_cast<uint64_t>(unsigned_left >> 64);
  const auto right_low = static_cast<uint64_t>(unsigned_right);
  const auto right_high = static_cast<uint64_t>(unsigned_right >> 64);

  uint64_t low_product_high = 0;
  uint64_t low_product_low = 0;
  umul64wide(left_low, right_low, &low_product_high, &low_product_low);

  const uint64_t result_high =
      low_product_high + (left_high * right_low) + (left_low * right_high);
  return static_cast<__int128>((static_cast<unsigned __int128>(result_high) << 64) |
                               low_product_low);
}

int __cxa_atexit(void (*)(void*), void*, void*) {
  return 0;
}

void __cxa_pure_virtual() {
  abort();
}

} // extern "C"

void* operator new(size_t size) {
  if (void* ptr = malloc(size)) {
    return ptr;
  }
  abort();
  __builtin_unreachable();
}

void* operator new[](size_t size) {
  return operator new(size);
}

void* operator new(size_t size, const std::nothrow_t&) noexcept {
  return malloc(size);
}

void* operator new(size_t size, std::align_val_t, const std::nothrow_t& tag) noexcept {
  return operator new(size, tag);
}

void operator delete(void* ptr) noexcept {
  free(ptr);
}

void operator delete[](void* ptr) noexcept {
  free(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
  free(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
  free(ptr);
}

void operator delete(void* ptr, size_t, std::align_val_t) noexcept {
  free(ptr);
}