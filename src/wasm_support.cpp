#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <poll.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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

char* strcpy(char* dest, const char* src) {
  size_t i = 0;
  do {
    dest[i] = src[i];
  } while (src[i++] != '\0');
  return dest;
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

char* strsignal(int) {
  return const_cast<char*>("signal");
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

char* strtok_r(char*, const char*, char**) {
  return nullptr;
}

int abs(int value) {
  return value < 0 ? -value : value;
}

int atoi(const char* str) {
  return static_cast<int>(strtol(str, nullptr, 10));
}

long labs(long value) {
  return value < 0 ? -value : value;
}

long long llabs(long long value) {
  return value < 0 ? -value : value;
}

div_t div(int numer, int denom) {
  return {numer / denom, numer % denom};
}

ldiv_t ldiv(long numer, long denom) {
  return {numer / denom, numer % denom};
}

lldiv_t lldiv(long long numer, long long denom) {
  return {numer / denom, numer % denom};
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

long double strtold(const char* str, char** end) {
  if (end != nullptr) {
    *end = const_cast<char*>(str);
  }
  return 0;
}

wchar_t* wcschr(const wchar_t*, wchar_t) {
  return nullptr;
}

int wmemcmp(const wchar_t* left, const wchar_t* right, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    if (left[i] != right[i]) {
      return left[i] < right[i] ? -1 : 1;
    }
  }
  return 0;
}

wchar_t* wmemcpy(wchar_t* dest, const wchar_t* src, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    dest[i] = src[i];
  }
  return dest;
}

wchar_t* wmemmove(wchar_t* dest, const wchar_t* src, size_t count) {
  if (dest < src) {
    for (size_t i = 0; i < count; ++i) {
      dest[i] = src[i];
    }
  } else if (dest > src) {
    for (size_t i = count; i > 0; --i) {
      dest[i - 1] = src[i - 1];
    }
  }
  return dest;
}

wchar_t* wmemset(wchar_t* dest, wchar_t value, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    dest[i] = value;
  }
  return dest;
}

size_t wcslen(const wchar_t* str) {
  size_t len = 0;
  while (str[len] != 0) {
    ++len;
  }
  return len;
}

int swprintf(wchar_t*, size_t, const wchar_t*, ...) {
  return -1;
}

wchar_t* wcspbrk(const wchar_t*, const wchar_t*) {
  return nullptr;
}

wchar_t* wcsrchr(const wchar_t*, wchar_t) {
  return nullptr;
}

wchar_t* wcsstr(const wchar_t*, const wchar_t*) {
  return nullptr;
}

wchar_t* wmemchr(const wchar_t*, wchar_t, size_t) {
  return nullptr;
}

long wcstol(const wchar_t* str, wchar_t** end, int) {
  if (end != nullptr) {
    *end = const_cast<wchar_t*>(str);
  }
  return 0;
}

unsigned long wcstoul(const wchar_t* str, wchar_t** end, int) {
  if (end != nullptr) {
    *end = const_cast<wchar_t*>(str);
  }
  return 0;
}

long long wcstoll(const wchar_t* str, wchar_t** end, int) {
  if (end != nullptr) {
    *end = const_cast<wchar_t*>(str);
  }
  return 0;
}

unsigned long long wcstoull(const wchar_t* str, wchar_t** end, int) {
  if (end != nullptr) {
    *end = const_cast<wchar_t*>(str);
  }
  return 0;
}

float wcstof(const wchar_t* str, wchar_t** end) {
  if (end != nullptr) {
    *end = const_cast<wchar_t*>(str);
  }
  return 0;
}

double wcstod(const wchar_t* str, wchar_t** end) {
  if (end != nullptr) {
    *end = const_cast<wchar_t*>(str);
  }
  return 0;
}

long double wcstold(const wchar_t* str, wchar_t** end) {
  if (end != nullptr) {
    *end = const_cast<wchar_t*>(str);
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

int feof(FILE*) {
  return 1;
}

int ferror(FILE*) {
  return 1;
}

int fflush(FILE*) {
  return 0;
}

int fprintf(FILE*, const char*, ...) {
  return -1;
}

int remove(const char*) {
  errno = ENOENT;
  return -1;
}

int rename(const char*, const char*) {
  errno = ENOENT;
  return -1;
}

int close(int) {
  errno = EBADF;
  return -1;
}

int access(const char*, int) {
  errno = ENOENT;
  return -1;
}

unsigned int alarm(unsigned int) {
  return 0;
}

int dup(int) {
  errno = EBADF;
  return -1;
}

int dup2(int, int) {
  errno = EBADF;
  return -1;
}

int execv(const char*, char* const[]) {
  errno = ENOENT;
  return -1;
}

int execve(const char*, char* const[], char* const[]) {
  errno = ENOENT;
  return -1;
}

void _exit(int) {
  abort();
}

int fork() {
  errno = ENOSYS;
  return -1;
}

int fcntl(int, int, ...) {
  errno = EBADF;
  return -1;
}

int open(const char*, int, ...) {
  errno = ENOENT;
  return -1;
}

int chdir(const char*) {
  errno = ENOENT;
  return -1;
}

char* getcwd(char* buffer, size_t size) {
  if (size > 0 && buffer != nullptr) {
    buffer[0] = '\0';
  }
  errno = ENOENT;
  return nullptr;
}

int gethostname(char* name, size_t len) {
  if (len > 0) {
    name[0] = '\0';
  }
  return 0;
}

int uname(struct utsname* name) {
  if (name != nullptr) {
    memset(name, 0, sizeof(*name));
    strncpy(name->sysname, "wasm", sizeof(name->sysname) - 1);
    strncpy(name->machine, "wasm32", sizeof(name->machine) - 1);
  }
  return 0;
}

pid_t getpid() {
  return 1;
}

unsigned int getuid() {
  return 0;
}

int isatty(int) {
  return 0;
}

int link(const char*, const char*) {
  errno = ENOENT;
  return -1;
}

int pipe(int[2]) {
  errno = EMFILE;
  return -1;
}

off_t lseek(int, off_t, int) {
  errno = EBADF;
  return -1;
}

ssize_t readlink(const char*, char*, size_t) {
  errno = ENOENT;
  return -1;
}

int rmdir(const char*) {
  errno = ENOENT;
  return -1;
}

pid_t setsid() {
  errno = ENOSYS;
  return -1;
}

long sysconf(int name) {
  if (name == _SC_PAGE_SIZE) {
    return 65536;
  }
  if (name == _SC_ARG_MAX) {
    return 4096;
  }
  errno = EINVAL;
  return -1;
}

int symlink(const char*, const char*) {
  errno = ENOSYS;
  return -1;
}

int ftruncate(int, off_t) {
  errno = EBADF;
  return -1;
}

int fchown(int, uid_t, gid_t) {
  errno = EBADF;
  return -1;
}

int unlink(const char*) {
  errno = ENOENT;
  return -1;
}

int usleep(unsigned int) {
  return 0;
}

int fstat(int, struct stat*) {
  errno = EBADF;
  return -1;
}

int stat(const char*, struct stat*) {
  errno = ENOENT;
  return -1;
}

int lstat(const char*, struct stat*) {
  errno = ENOENT;
  return -1;
}

int mkdir(const char*, mode_t) {
  errno = ENOSYS;
  return -1;
}

int chmod(const char*, mode_t) {
  errno = ENOENT;
  return -1;
}

int fchmod(int, mode_t) {
  errno = EBADF;
  return -1;
}

mode_t umask(mode_t) {
  return 0;
}

int poll(struct pollfd*, nfds_t, int) {
  errno = EBADF;
  return -1;
}

void* mmap(void*, size_t, int, int, int, off_t) {
  errno = ENOMEM;
  return MAP_FAILED;
}

int munmap(void*, size_t) {
  return 0;
}

int mprotect(void*, size_t, int) {
  errno = ENOMEM;
  return -1;
}

int msync(void*, size_t, int) {
  errno = ENOMEM;
  return -1;
}

int madvise(void*, size_t, int) {
  return 0;
}

int statvfs(const char*, struct statvfs*) {
  errno = ENOENT;
  return -1;
}

int fstatvfs(int, struct statvfs*) {
  errno = EBADF;
  return -1;
}

int getrlimit(int, struct rlimit* limit) {
  if (limit != nullptr) {
    limit->rlim_cur = 8ul << 20;
    limit->rlim_max = 8ul << 20;
  }
  return 0;
}

int setrlimit(int, const struct rlimit*) {
  return 0;
}

int getrusage(int, struct rusage* usage) {
  if (usage != nullptr) {
    memset(usage, 0, sizeof(*usage));
  }
  return 0;
}

int socket(int, int, int) {
  errno = ENOSYS;
  return -1;
}

int connect(int, const struct sockaddr*, socklen_t) {
  errno = ENOSYS;
  return -1;
}

int bind(int, const struct sockaddr*, socklen_t) {
  errno = ENOSYS;
  return -1;
}

int listen(int, int) {
  errno = ENOSYS;
  return -1;
}

int accept(int, struct sockaddr*, socklen_t*) {
  errno = ENOSYS;
  return -1;
}

int setsockopt(int, int, int, const void*, socklen_t) {
  errno = ENOSYS;
  return -1;
}

DIR* opendir(const char*) {
  errno = ENOENT;
  return nullptr;
}

struct dirent* readdir(DIR*) {
  errno = EBADF;
  return nullptr;
}

int closedir(DIR*) {
  errno = EBADF;
  return -1;
}

struct passwd* getpwuid(unsigned int) {
  return nullptr;
}

int getpwnam_r(const char*, struct passwd*, char*, unsigned long, struct passwd** result) {
  if (result != nullptr) {
    *result = nullptr;
  }
  return 0;
}

int getpwuid_r(unsigned int, struct passwd*, char*, unsigned long, struct passwd** result) {
  if (result != nullptr) {
    *result = nullptr;
  }
  return 0;
}

void* dlopen(const char*, int) {
  return nullptr;
}

void* dlsym(void*, const char*) {
  return nullptr;
}

int dlclose(void*) {
  return 0;
}

char* dlerror(void) {
  return nullptr;
}

int dladdr(const void*, Dl_info* info) {
  if (info != nullptr) {
    memset(info, 0, sizeof(*info));
  }
  return 0;
}

pid_t getsid(pid_t) {
  errno = ESRCH;
  return -1;
}

ssize_t read(int, void*, size_t) {
  errno = EBADF;
  return -1;
}

ssize_t write(int, const void*, size_t) {
  errno = EBADF;
  return -1;
}

time_t time(time_t* out) {
  if (out != nullptr) {
    *out = 0;
  }
  return 0;
}

int gettimeofday(struct timeval* tv, void*) {
  if (tv != nullptr) {
    tv->tv_sec = 0;
    tv->tv_usec = 0;
  }
  return 0;
}

struct tm* gmtime_r(const time_t*, struct tm* result) {
  if (result != nullptr) {
    memset(result, 0, sizeof(*result));
  }
  return result;
}

struct tm* localtime_r(const time_t*, struct tm* result) {
  return gmtime_r(nullptr, result);
}

struct tm* gmtime(const time_t* timep) {
  static struct tm result;
  return gmtime_r(timep, &result);
}

struct tm* localtime(const time_t* timep) {
  return gmtime(timep);
}

unsigned long strftime(char* buffer, unsigned long size, const char*, const struct tm*) {
  if (size > 0) {
    buffer[0] = '\0';
  }
  return 0;
}

sighandler_t signal(int, sighandler_t handler) {
  return handler;
}

int sigemptyset(sigset_t* set) {
  if (set != nullptr) {
    *set = 0;
  }
  return 0;
}

int sigfillset(sigset_t* set) {
  if (set != nullptr) {
    *set = ~0ul;
  }
  return 0;
}

int sigaddset(sigset_t* set, int signum) {
  if (set != nullptr) {
    *set |= (1ul << static_cast<unsigned>(signum));
  }
  return 0;
}

int sigaction(int, const struct sigaction*, struct sigaction*) {
  return 0;
}

int sigprocmask(int, const sigset_t*, sigset_t*) {
  return 0;
}

int raise(int) {
  return 0;
}

int kill(pid_t, int) {
  errno = ESRCH;
  return -1;
}

int waitpid(int, int*, int) {
  errno = ESRCH;
  return -1;
}

pid_t wait4(pid_t, int*, int, struct rusage*) {
  errno = ESRCH;
  return -1;
}

int wait(int*) {
  errno = ESRCH;
  return -1;
}

void longjmp(jmp_buf, int) {
  abort();
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

void* operator new[](size_t size, const std::nothrow_t& tag) noexcept {
  return operator new(size, tag);
}

void* operator new(size_t size, std::align_val_t) {
  return operator new(size);
}

void* operator new[](size_t size, std::align_val_t alignment) {
  return operator new(size, alignment);
}

void* operator new(size_t size, std::align_val_t, const std::nothrow_t& tag) noexcept {
  return operator new(size, tag);
}

void* operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept {
  return operator new(size, alignment, tag);
}

void operator delete(void* ptr) noexcept {
  free(ptr);
}

void operator delete[](void* ptr) noexcept {
  free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
  free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
  free(ptr);
}

void operator delete(void* ptr, std::align_val_t) noexcept {
  free(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept {
  free(ptr);
}

void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept {
  free(ptr);
}

void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept {
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

void operator delete[](void* ptr, size_t, std::align_val_t) noexcept {
  free(ptr);
}

namespace std {

void __libcpp_verbose_abort(char const*, ...) noexcept {
  __builtin_trap();
}

} // namespace std
