/******************************************************************************
 *
 *                               The NG Project
 *
 *                            Bind the socket injector
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <dlfcn.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ligHT.h"
#include "socketinfo.h"
#include "actions.h"
#include "conffile.h"
#include "runtime.h"

#define DEFAULT_CONFIG "libinject.rules"

void inj_init(void) __attribute__((constructor));
void inj_fini(void) __attribute__((destructor));


/*** Keeping informations about the status of file descriptors */

static LigHT* sockets = NULL;


/*** Callbacks pointing to the overriden system function */

typedef ssize_t (readfun)(int fd, void* buf, size_t len);
typedef ssize_t (recvfun)(int fd, void* buf, size_t len, int flags);
typedef ssize_t (recvfromfun)(int fd, void* __restrict buf, size_t n, int flags,
                               __SOCKADDR_ARG addr, socklen_t* __restrict addr_len);
typedef ssize_t (recvmsgfun)(int fd, struct msghdr* message, int flags);

typedef ssize_t (writefun)(int fd, __const void* buf, size_t n);
typedef ssize_t (sendfun)(int fd, __const void* buf, size_t n, int flags);
typedef ssize_t (sendtofun)(int fd, __const void* buf, size_t n, int flags,
                            __CONST_SOCKADDR_ARG addr, socklen_t addr_len);
typedef ssize_t (sendmsgfun)(int fd, __const struct msghdr* message, int flags);

typedef int (connectfun)(int fd, __CONST_SOCKADDR_ARG addr, socklen_t addrlen);
typedef ssize_t (closefun)(int fd);

static readfun*     sysread;
static recvfun*     sysrecv;
static recvfromfun* sysrecvfrom;
static recvmsgfun*  sysrecvmsg;

static writefun*    syswrite;
static sendfun*     syssend;
static sendtofun*   syssendto;
static sendmsgfun*  syssendmsg;

static connectfun*  sysconnect;
static closefun*    sysclose;

static Config* config = NULL;


/*** Useful stuff */

/** Data to pass when performing syscalls that fetch addr.
 */
struct AddrData {
   __SOCKADDR_ARG addr; /**< Source address. */
   socklen_t* addr_len; /**< Length of the source address. */
};

/** Data to pass when performing syscalls that use addr.
 */
struct ConstAddrData {
   __CONST_SOCKADDR_ARG addr; /**< Destination address. */
   socklen_t addr_len;        /**< Destination address length. */
};

#define RELEASE_SI                                                             \
  if (si) {                                                                    \
    SocketInfo_unlock(si);                                                     \
  }

/** Get informations about the socket.
 */
static inline SocketInfo* getInfos(int fd) {
  SocketInfo* si = NULL;
  if (!config || !sockets) {
    return NULL;
  }
  si = (SocketInfo*)LigHT_get(sockets, fd, true);
  si = SocketInfo_check(si, fd);
  if (si == NULL || si->toDestroy) {
    if ((si = SocketInfo_init(fd))) {
      SocketInfo_lock(si);
    }
    (void)LigHT_put(sockets, fd, si, true, true);
  }
  return si;
}

/** Get informations about the socket while connecting.
 */
static inline SocketInfo* getInfosOnConnect(int fd, const struct ConstAddrData* data) {
  SocketInfo* si = NULL;
  SocketInfo* old = NULL;
  if (!sockets) {
    return NULL;
  }
  si = SocketInfo_initLight(fd, data->addr, data->addr_len);
  SocketInfo_lock(si);
  old = (SocketInfo*)LigHT_get(sockets, fd, true);
  if (old) {
    SocketInfo_lock(old);
    old->toDestroy = true;
    SocketInfo_unlock(old);
  }
  (void)LigHT_put(sockets, fd, si, true, true);
  return si;
}


/*** Receiving calls */

/* read */

static ssize_t readCB(int fd, void* buf, size_t len, int flags, void* data) {
  return sysread(fd, buf, len);
}

ssize_t read(int fd, void* buf, size_t len) {
  SocketInfo* si = NULL;
  ssize_t ret;

  si = getInfos(fd);
  if (si) {
    ret = ActionQueue_process(config->queue, si, Reading, readCB, buf, len, 0, NULL);
  } else {
    ret = sysread(fd, buf, len);
  }
  RELEASE_SI
  return ret;
}


/* recv */

static ssize_t recvCB(int fd, void* buf, size_t len, int flags, void* data) {
  return sysrecv(fd, buf, len, flags);
}

ssize_t recv(int fd, void* buf, size_t len, int flags) {
  SocketInfo* si = NULL;
  ssize_t ret;

  si = getInfos(fd);
  if (si) {
    ret = ActionQueue_process(config->queue, si, Reading, recvCB, buf, len, flags, NULL);
  } else {
    ret = sysrecv(fd, buf, len, flags);
  }
  RELEASE_SI
  return ret;
}

/* recvfrom */

static ssize_t recvfromCB(int fd, void* buf, size_t len, int flags, void* data) {
  struct AddrData* d = (struct AddrData*)data;
  return sysrecvfrom(fd, buf, len, flags, d->addr, d->addr_len);
}

ssize_t recvfrom(int fd, void* __restrict buf, size_t len, int flags,
                 __SOCKADDR_ARG addr, socklen_t* __restrict addr_len) {
  SocketInfo* si;
  ssize_t ret;
  struct AddrData d;
  d.addr     = addr;
  d.addr_len = addr_len;

  si = getInfos(fd);
  if (si) {
    ret = ActionQueue_process(config->queue, si, Reading, recvfromCB, buf, len, flags, &d);
  } else {
    ret = sysrecvfrom(fd, buf, len, flags, addr, addr_len);
  }
  RELEASE_SI
  return ret;
}

#if 0 /* Not yet implemented. Should be done to be sure to catch all messages. */
ssize_t recvmsg(int fd, struct msghdr* message, int flags) {
  return sysrecvmsg(fd, message, flags);
}
#endif


/*** Sending calls */

/* write */

static ssize_t writeCB(int fd, void* buf, size_t len, int flags, void* data) {
  return syswrite(fd, buf, len);
}

ssize_t write(int fd, __const void* buf, size_t n) {
  SocketInfo* si;
  ssize_t ret;

  si = getInfos(fd);
  if (si) {
    ret = ActionQueue_process(config->queue, si, Writing, writeCB, (void*)buf, n, 0, NULL);
  } else {
    ret = syswrite(fd, buf, n);
  }
  RELEASE_SI
  return ret;
}


/* send */

static ssize_t sendCB(int fd, void* buf, size_t len, int flags, void* data) {
  return syssend(fd, buf, len, flags);
}

ssize_t send(int fd, __const void* buf, size_t n, int flags) {
  SocketInfo* si;
  ssize_t ret;

  si = getInfos(fd);
  if (si) {
    ret = ActionQueue_process(config->queue, si, Writing, sendCB, (void*)buf, n, flags, NULL);
  } else {
    ret = syssend(fd, buf, n, flags);
  }
  RELEASE_SI
  return ret;
}


/* sendto */

static ssize_t sendtoCB(int fd, void* buf, size_t len, int flags, void* data) {
  struct ConstAddrData* d = (struct ConstAddrData*)data;
  return syssendto(fd, buf, len, flags, d->addr, d->addr_len);
}

ssize_t sendto(int fd, __const void* buf, size_t n, int flags,
               __CONST_SOCKADDR_ARG addr, socklen_t addr_len) {
  SocketInfo* si;
  ssize_t ret;
  struct ConstAddrData d;
  d.addr     = addr;
  d.addr_len = addr_len;

  si = getInfos(fd);
  if (si) {
    ret = ActionQueue_process(config->queue, si, Writing, sendtoCB, (void*)buf, n, flags, &d);
  } else {
    ret = syssendto(fd, buf, n, flags, addr, addr_len);
  }
  RELEASE_SI
  return ret;
}

#if 0 /* Not yet implemented, should be done to be sure to catch all messages. */
ssize_t sendmsg(int fd, __const struct msghdr* message, int flags) {
  return syssendmsg(fd, message, flags);
}
#endif


/*** Want to connect to a new socket... */

static ssize_t connectCB(int fd, void* buf, size_t len, int flags, void* data) {
  struct ConstAddrData* d = (struct ConstAddrData*)data;
  return sysconnect(fd, d->addr, d->addr_len);
}

int connect(int fd, __CONST_SOCKADDR_ARG addr, socklen_t addrlen) {
  SocketInfo* si;
  struct ConstAddrData d;
  int ret;
  d.addr     = addr;
  d.addr_len = addrlen;

  si = getInfosOnConnect(fd, &d);
  if (si) {
    ret = ActionQueue_process(config->queue, si, Connecting, connectCB, NULL, 0, 0, &d);
  } else {
    ret = sysconnect(fd, addr, addrlen);
  }
  RELEASE_SI
  return ret;
}


/*** The socket has been closed... */

static ssize_t closeCB(int fd, void* buf, size_t len, int flags, void* data) {
  return sysclose(fd);
}

int close(int fd) {
  SocketInfo* si;
  int ret;

  si = getInfos(fd);
  if (si) {
    LigHT_remove(sockets, fd, true);
    ret = ActionQueue_process(config->queue, si, Closing, closeCB, NULL, 0, 0, NULL);
    si->toDestroy = true;
  } else {
    ret = sysclose(fd);
  }
  RELEASE_SI
  return ret;
}


/*** Lib initialisation */

/** Start the module.
 */
void inj_init(void) {
  srand(time(NULL));

  sysread     = (readfun*)dlsym(RTLD_NEXT, "read");
  sysrecv     = (recvfun*)dlsym(RTLD_NEXT, "recv");
  sysrecvfrom = (recvfromfun*)dlsym(RTLD_NEXT, "recvfrom");
  sysrecvmsg  = (recvmsgfun*)dlsym(RTLD_NEXT, "recvmsg");

  syswrite    = (writefun*)dlsym(RTLD_NEXT, "write");
  syssend     = (sendfun*)dlsym(RTLD_NEXT, "send");
  syssendto   = (sendtofun*)dlsym(RTLD_NEXT, "sendto");
  syssendmsg  = (sendmsgfun*)dlsym(RTLD_NEXT, "sendmsg");

  sysconnect  = (connectfun*)dlsym(RTLD_NEXT, "connect");
  sysclose    = (closefun*)dlsym(RTLD_NEXT, "close");
  ActionSet_init();

  if (getenv("LIBINJ_DISABLE")) {
    config  = NULL;
    sockets = NULL;
  } else {
    sockets = LigHT_init(16380, NULL);
    if ((config = Config_init(getenv("LIBINJ_CONFIG"))) == NULL) {
      config = Config_init(DEFAULT_CONFIG);
    }
  }
  if (config) {
    Runtime_start(config);
  }
}

/** Close the module and free all used memory.
 */
void inj_fini(void) {
  if (sockets) {
    LigHT_destroy(sockets);
  }
  if (config) {
    Config_destroy(config);
  }
  sockets = NULL;
}
