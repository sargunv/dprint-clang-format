#ifndef DPRINT_CLANG_FORMAT_WASM_SYS_SOCKET_H
#define DPRINT_CLANG_FORMAT_WASM_SYS_SOCKET_H

#include <stddef.h>
#include <sys/types.h>

typedef unsigned int socklen_t;

struct sockaddr {
  unsigned short sa_family;
  char sa_data[14];
};

#define AF_UNIX 1
#define SOCK_STREAM 1
#define SOL_SOCKET 1
#define SO_PEERCRED 17

#ifdef __cplusplus
extern "C" {
#endif

int socket(int domain, int type, int protocol);
int connect(int socket, const struct sockaddr *address, socklen_t address_len);
int bind(int socket, const struct sockaddr *address, socklen_t address_len);
int listen(int socket, int backlog);
int accept(int socket, struct sockaddr *address, socklen_t *address_len);
int setsockopt(int socket, int level, int option_name, const void *option_value,
               socklen_t option_len);

#ifdef __cplusplus
}
#endif

#endif
