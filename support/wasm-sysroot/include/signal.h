#ifndef DPRINT_CLANG_FORMAT_WASM_SIGNAL_H
#define DPRINT_CLANG_FORMAT_WASM_SIGNAL_H

#include <sys/types.h>

#define SIGABRT 6
#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGSEGV 11
#define SIGBUS 10
#define SIGILL 4
#define SIGFPE 8
#define SIGTRAP 5
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGKILL 9
#define SIGUSR1 30
#define SIGUSR2 31
#define SIG_SETMASK 2
#define SIG_UNBLOCK 1

#define SA_NODEFER 0x01
#define SA_RESETHAND 0x02
#define SA_ONSTACK 0x04
#define SA_SIGINFO 0x08

typedef void (*sighandler_t)(int);
typedef unsigned long sigset_t;

typedef struct {
  int si_signo;
  int si_code;
  pid_t si_pid;
} siginfo_t;

struct sigaction {
  sighandler_t sa_handler;
  void (*sa_sigaction)(int, siginfo_t*, void*);
  sigset_t sa_mask;
  int sa_flags;
};

#ifdef __cplusplus
extern "C" {
#endif

sighandler_t signal(int signum, sighandler_t handler);
int sigemptyset(sigset_t* set);
int sigfillset(sigset_t* set);
int sigaddset(sigset_t* set, int signum);
int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact);
int sigprocmask(int how, const sigset_t* set, sigset_t* oldset);
int raise(int signum);
int kill(pid_t pid, int signum);

#ifdef __cplusplus
}
#endif

#endif
