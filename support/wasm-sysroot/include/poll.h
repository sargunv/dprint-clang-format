#ifndef DPRINT_CLANG_FORMAT_WASM_POLL_H
#define DPRINT_CLANG_FORMAT_WASM_POLL_H

typedef unsigned long nfds_t;

struct pollfd {
  int fd;
  short events;
  short revents;
};

#define POLLIN 0x001
#define POLLOUT 0x004
#define POLLERR 0x008
#define POLLNVAL 0x020

#ifdef __cplusplus
extern "C" {
#endif

int poll(struct pollfd* fds, nfds_t nfds, int timeout);

#ifdef __cplusplus
}
#endif

#endif
