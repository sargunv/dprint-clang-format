#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_WAIT_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_WAIT_H

#include <sys/resource.h>
#include <sys/types.h>

#define WNOHANG 1
#define WIFEXITED(status) (1)
#define WEXITSTATUS(status) (status)
#define WIFSIGNALED(status) (0)
#define WTERMSIG(status) (0)

#ifdef __cplusplus
extern "C" {
#endif

int waitpid(int pid, int *status, int options);
pid_t wait4(pid_t pid, int *status, int options, struct rusage *usage);
int wait(int *status);

#ifdef __cplusplus
}
#endif

#endif
