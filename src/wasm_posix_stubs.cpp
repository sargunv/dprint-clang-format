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


} // extern "C"
