/******************************************************************************
 *
 *                               The NG Project
 *
 *                    Stuff to retreive info from sockets
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#ifndef _SOCKET_INFO_H_
#define _SOCKET_INFO_H_

#include <sys/socket.h>
#include <stdint.h>

/** @defgroup SocketInfo Socket informations
 *
 * A socket information structure stores informations related to a given socket.
 * These informations are connection informations (socket, ports, addresses) and
 * a user defined data that can store any kind of structure. @{
 */

/** A socket info.
 */
typedef struct SocketInfo SocketInfo;

/** Callback called when a SocketInfo is beeing destroyed.
 *
 * This callback MUST free the user data associated with the given SocketInfo.
 *
 * @param si The socket info to clean.
 */
typedef void (SocketInfoDataFree)(SocketInfo* si);

/** Helper for storing data direction.
 */
typedef enum {
  Reading    = 1,                    /**< Reading data on the socket. */
  Writing    = 2,                    /**< Writing data to the socket. */
  Connecting = 4,                    /**< Connecting a socket. */
  Closing    = 8,                    /**< Closing a socket. */
  Data       = Reading | Writing,    /**< Data operation (read or write). */
  Connection = Connecting | Closing, /**< Operation impacting connection state. */
  Any_Dir    = Data | Connection     /**< Any operation. */
} SocketInfoDirection;

/** Protocole.
 */
typedef enum Proto {
  AP_TCP = 1,               /**< TCP socket */
  AP_UDP = 2,               /**< UDP socket */
  AP_IP  = AP_TCP | AP_UDP  /**< IP socket (TCP or UDP) */
} Proto;

/** Host type.
 */
typedef enum Host {
  AH_None    = 0,  /**< I am nobody. */
  AH_Me      = 1,  /**< I am ze host. */
  AH_DNS     = 2,  /**< I love DNS. */
  AH_Address = 4,  /**< Let's give me your IP address and I'll start hacking you. */
  AH_Any     = AH_Me | AH_DNS | AH_Address /**< Any address */
} Host;

/** Struct HostAddress.
 */
typedef struct HostAddress {
  enum Host type; /**< The kind of host. */
  uint32_t  addr; /**< IP address if the kind of host requires it. */
  int       port; /**< The host port (or -1 if all the port are matched). */
} HostAddress;

/** Definition of the SocketInfo structure
 */
struct SocketInfo {
  int         fd;             /**< File descriptor of the socket. */
  Proto       proto;          /**< The socket is TCP. */
  HostAddress local;          /**< Local address. */
  HostAddress remote;         /**< Remote address. */
  bool        blocking;       /**< If true, the socket is blocking. */

  bool        toDestroy;      /**< If true, the info can be destroied. */
  int         sem;            /**< Number of accessors. */
  pthread_mutex_t semLock;    /**< Lock to access the number of accessors. */

  void* data;                 /**< User data associated with the socket. */
  SocketInfoDataFree* free;   /**< Callback used to clear the user data. */
};


/** Build a new socket info.
 *
 * Fetch informations about the given socket and fill a light SocketInfo
 * structure.
 *
 * @param fd File descriptor of the socket.
 * @return A new SocketInfo structure or NULL if the fd is not a valid socket.
 */
SocketInfo* SocketInfo_init(int fd);

/** Build a new socket info from data given by the user.
 *
 * This won't fetch informations about the given socket but only from
 * the sockaddr given by the user. This is useful to build a temporary
 * SocketInfo for connect or other calls that are performed before the
 * socket is connected.
 *
 * @param fd File descriptor of the socket
 * @param addr The address to which the socket should be connected.
 * @param addrlen The length of the addr structure.
 * @return A new SocketInfo structure or NULL if the fd is not a valid socket.
 */
SocketInfo* SocketInfo_initLight(int fd, __CONST_SOCKADDR_ARG addr, socklen_t addrlen);

/** Free the content of a socket info.
 *
 * @param si SocketInfo to free
 */
void SocketInfo_destroy(SocketInfo* si);

/** Set socket info user data and register the corresponding remover.
 *
 * @param si   The socket infos.
 * @param data Data to be associated with the socket info
 * @param cb   Remover callback
 */
void SocketInfo_setData(SocketInfo* si, void* data, SocketInfoDataFree* cb);

/** Register a new user.
 *
 * @param si The socket info.
 */
void SocketInfo_lock(SocketInfo* si);

/** Unregister a user.
 *
 * If the toDestroy flag has been set, the socket info will be destroyed.
 *
 * @param si The socket info.
 */
void SocketInfo_unlock(SocketInfo* si);

/** @} */

#endif
