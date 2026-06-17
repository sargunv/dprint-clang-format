#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

extern "C" {

extern int errno;
void abort();
void* malloc(size_t);
void free(void*);
void* memset(void*, int, size_t);
void* memcpy(void*, const void*, size_t);
char* strncpy(char*, const char*, size_t);

int remove(const char*) {
  errno = ENOENT;
  return -1;
}

int rename(const char*, const char*) {
  errno = ENOENT;
  return -1;
}

unsigned int alarm(unsigned int) {
  return 0;
}

int fcntl(int, int, ...) {
  errno = EBADF;
  return -1;
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

int link(const char*, const char*) {
  errno = ENOENT;
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

int usleep(unsigned int) {
  return 0;
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

int mprotect(void*, size_t, int) {
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

int getpwnam_r(const char*, struct passwd*, char*, unsigned long, struct passwd** result) {
  if (result != nullptr) {
    *result = nullptr;
  }
  return 0;
}

pid_t getsid(pid_t) {
  errno = ESRCH;
  return -1;
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

unsigned long strftime(char* buffer, unsigned long size, const char*, const struct tm*) {
  if (size > 0) {
    buffer[0] = '\0';
  }
  return 0;
}

} // extern "C"
