#ifndef DPRINT_CLANG_FORMAT_WASM_UNISTD_H
#define DPRINT_CLANG_FORMAT_WASM_UNISTD_H

#include <sys/types.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

#define _SC_PAGE_SIZE 30
#define _SC_ARG_MAX 0
#define _SC_GETPW_R_SIZE_MAX 1
#define _POSIX_ARG_MAX 4096

#ifdef __cplusplus
extern "C" {
#endif

int close(int fd);
int access(const char *path, int mode);
unsigned int alarm(unsigned int seconds);
int dup(int fd);
int dup2(int oldfd, int newfd);
int chdir(const char *path);
int execv(const char *path, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);
void _exit(int status) __attribute__((noreturn));
int fork(void);
char *getcwd(char *buffer, size_t size);
int gethostname(char *name, size_t len);
pid_t getpid(void);
unsigned int getuid(void);
pid_t getsid(pid_t pid);
int isatty(int fd);
int link(const char *old_path, const char *new_path);
int pipe(int fds[2]);
off_t lseek(int fd, off_t offset, int whence);
ssize_t read(int fd, void *buffer, size_t count);
ssize_t readlink(const char *path, char *buffer, size_t size);
int rmdir(const char *path);
pid_t setsid(void);
long sysconf(int name);
int symlink(const char *target, const char *link_path);
int ftruncate(int fd, off_t length);
int fchown(int fd, uid_t owner, gid_t group);
int unlink(const char *path);
int usleep(unsigned int usec);
ssize_t write(int fd, const void *buffer, size_t count);

#ifdef __cplusplus
}
#endif

#endif
