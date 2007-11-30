/******************************************************************************
 *
 *                               The NG Project
 *
 *                      Stuff to retreive info from sockets
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "socketinfo.h"

typedef int (getsockinfofun)(int s, struct sockaddr* name, socklen_t* namelen);


static inline bool SocketInfo_fetchData(int fd, getsockinfofun* callback,
                                        HostAddress* host) {
  struct sockaddr_in saddr;
  socklen_t size = sizeof(struct sockaddr_in);

  if (callback(fd, (struct sockaddr*)&saddr, &size) == -1) {
    return false;
  }
  if (saddr.sin_family != AF_INET) {
    return false;
  }
  host->addr = ntohl(saddr.sin_addr.s_addr);
  host->port = ntohs(saddr.sin_port);
  return true;
}

static SocketInfo* SocketInfo_setup(SocketInfo* si, int fd, int err) {
  int type = 0;
  socklen_t len = sizeof(int);

  if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) == -1 ||
      (type != SOCK_STREAM && type != SOCK_DGRAM)) {
    free(si);
    errno = err;
    return NULL;
  }
  si->proto = type == SOCK_STREAM ? AP_TCP : AP_UDP;
  if ((type = fcntl(fd, F_GETFL, 0)) == -1) {
    free(si);
    errno = err;
    return NULL;
  }
  si->local.type = AH_Me;
  si->remote.type = (si->remote.port == 53 || si->local.port == 53) ? AH_DNS : AH_Address;
  si->blocking = !(type & O_NONBLOCK);
  si->fd   = fd;
  si->data = NULL;
  si->free = NULL;
  pthread_mutex_init(&si->semLock, NULL);
  si->sem  = 0;
  si->toDestroy = false;
  errno = err;
  return si;
}

SocketInfo* SocketInfo_init(int fd) {
  SocketInfo* si;
  int err = errno;
  struct stat s;

  if (fstat(fd, &s) == -1 || !S_ISSOCK(s.st_mode)) {
    errno = err;
    return NULL;
  }

  si = (SocketInfo*)malloc(sizeof(SocketInfo));
  if (!SocketInfo_fetchData(fd, getpeername, &si->remote)
      || !SocketInfo_fetchData(fd, getsockname, &si->local)) {
    free(si);
    errno = err;
    return NULL;
  }
  return SocketInfo_setup(si, fd, err);
}

SocketInfo* SocketInfo_initLight(int fd, __CONST_SOCKADDR_ARG addr, socklen_t addrlen) {
  SocketInfo* si;
  int err = errno;
  struct stat s;
  struct sockaddr_in* saddr = (struct sockaddr_in*)addr;

  if (fstat(fd, &s) == -1 || !S_ISSOCK(s.st_mode)) {
    errno = err;
    return NULL;
  }
  si = (SocketInfo*)malloc(sizeof(SocketInfo));
  si->remote.addr = ntohl(saddr->sin_addr.s_addr);
  si->remote.port = ntohs(saddr->sin_port);
  si->local.addr  = 0;
  si->local.port  = 0;
  return SocketInfo_setup(si, fd, err);
}

void SocketInfo_destroy(SocketInfo* si) {
  if (!si) {
    return;
  }
  if (si->free) {
    si->free(si);
  }
  pthread_mutex_destroy(&si->semLock);
  free(si);
}

#include <stdio.h>

SocketInfo* SocketInfo_check(SocketInfo* si, int fd) {
  int type = 0;
  socklen_t len = sizeof(int);
  int err = errno;

#define RELEASE_SI                                                             \
  {                                                                            \
    bool toDestroy = false;                                                    \
    si->toDestroy  = true;                                                     \
    pthread_mutex_lock(&si->semLock);                                          \
    toDestroy = (si->sem == 0);                                                \
    pthread_mutex_unlock(&si->semLock);                                        \
    if (toDestroy) {                                                           \
      SocketInfo_destroy(si);                                                  \
    }                                                                          \
  }                                                                            \
  errno = err;                                                                 \
  return NULL;

  if (si == NULL) {
    return NULL;
  }
  if (si->proto == AP_UDP &&
      (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) == -1
       || (type != SOCK_STREAM && type != SOCK_DGRAM)
       || (si->proto != (type == SOCK_STREAM ? AP_TCP : AP_UDP)))) {
    free(si);
    RELEASE_SI
  }
  if (si->local.port == 0 && !SocketInfo_fetchData(fd, getsockname, &si->local)) {
    RELEASE_SI
  }
  errno = err;

#undef RELEASE_SI
  return si;
}

void SocketInfo_setData(SocketInfo* si, void* data, SocketInfoDataFree* cb) {
  si->data = data;
  si->free = cb;
}

void SocketInfo_lock(SocketInfo* si) {
  pthread_mutex_lock(&si->semLock);
  ++si->sem;
  pthread_mutex_unlock(&si->semLock);
}

void SocketInfo_unlock(SocketInfo* si) {
  bool toDestroy;
  pthread_mutex_lock(&si->semLock);
  si->sem--;
  toDestroy = si->toDestroy && si->sem == 0;
  pthread_mutex_unlock(&si->semLock);
  if (toDestroy) {
    SocketInfo_destroy(si);
  }
}
