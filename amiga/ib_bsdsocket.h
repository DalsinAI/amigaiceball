/* Iceball's POSIX socket calls on the Amiga's bsdsocket.library (Roadshow,
 * AmiTCP, Miami or GFOS's own stack), IPv4 only. Included by common.h (our
 * patch) in place of the POSIX socket headers. bsdsocket.library is opened
 * by ib_amiga.c when there is one; without a stack every call fails and
 * Iceball's own local game still runs (it needs no socket).
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#ifndef IB_BSDSOCKET_H
#define IB_BSDSOCKET_H

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include <exec/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>

extern struct Library *SocketBase;
#include <proto/bsdsocket.h>

/* Every socket call goes through ib_amiga.c, which fails it (-1, errno
 * ENETDOWN) when no stack is running, instead of calling a library that is
 * not open. close() and fcntl() here are for sockets only, as in Iceball and
 * ENet. */

/* no IPv6: the IPv6 socket Iceball also opens just fails */
#ifndef AF_INET6
#define AF_INET6 10
#endif
struct ib_in6_addr { unsigned char s6_addr[16]; };
struct sockaddr_in6 {
	unsigned char sin6_len;
	unsigned char sin6_family;
	unsigned short sin6_port;
	unsigned long sin6_flowinfo;
	struct ib_in6_addr sin6_addr;
	unsigned long sin6_scope_id;
};
#define in6_addr ib_in6_addr
extern const struct ib_in6_addr ib_in6addr_any;
#define in6addr_any ib_in6addr_any

struct sockaddr_storage {
	unsigned char ss_len;
	unsigned char ss_family;
	char ss_pad[126];
};

/* getaddrinfo, IPv4 only, on gethostbyname */
#ifndef AI_PASSIVE
#define AI_PASSIVE 1
#endif
#ifndef EAI_NONAME
#define EAI_NONAME 8
#endif
#ifndef EAI_FAIL
#define EAI_FAIL 4
#endif
#ifndef EAI_MEMORY
#define EAI_MEMORY 6
#endif
struct addrinfo {
	int ai_flags;
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	socklen_t ai_addrlen;
	struct sockaddr *ai_addr;
	char *ai_canonname;
	struct addrinfo *ai_next;
};
#ifndef IB_AMIGA_C
/* the stack's inline calls give way to ours */
#undef socket
#undef connect
#undef bind
#undef listen
#undef accept
#undef send
#undef recv
#undef sendto
#undef recvfrom
#undef setsockopt
#undef getsockopt
#undef getsockname
#undef shutdown
#undef sendmsg
#undef recvmsg
#undef select
#undef ioctl
#undef close
#undef fcntl
#undef gethostbyname
#undef gethostbyaddr
#undef inet_aton
#undef inet_ntoa
#undef inet_addr
#undef getaddrinfo
#undef freeaddrinfo
#undef gai_strerror
#undef inet_ntop
#undef inet_pton
#define socket ib_socket
#define connect ib_connect
#define bind ib_bind
#define listen ib_listen
#define accept ib_accept
#define send ib_send
#define recv ib_recv
#define sendto ib_sendto
#define recvfrom ib_recvfrom
#define setsockopt ib_setsockopt
#define getsockopt ib_getsockopt
#define getsockname ib_getsockname
#define shutdown ib_shutdown
#define sendmsg ib_sendmsg
#define recvmsg ib_recvmsg
#define select ib_select
#define ioctl ib_ioctl
#define close ib_close
#define fcntl ib_fcntl
#define gethostbyname ib_gethostbyname
#define gethostbyaddr ib_gethostbyaddr
#define inet_aton ib_inet_aton
#define inet_ntoa ib_inet_ntoa
#define inet_addr ib_inet_addr
#define getaddrinfo ib_getaddrinfo
#define freeaddrinfo ib_freeaddrinfo
#define gai_strerror ib_gai_strerror
#define inet_ntop ib_inet_ntop
#define inet_pton ib_inet_pton
#endif

int ib_socket(int domain, int type, int protocol);
int ib_connect(int s, const struct sockaddr *a, socklen_t n);
int ib_bind(int s, const struct sockaddr *a, socklen_t n);
int ib_listen(int s, int backlog);
int ib_accept(int s, struct sockaddr *a, socklen_t *n);
int ib_send(int s, const void *b, size_t n, int flags);
int ib_recv(int s, void *b, size_t n, int flags);
int ib_sendto(int s, const void *b, size_t n, int flags, const struct sockaddr *a, socklen_t an);
int ib_recvfrom(int s, void *b, size_t n, int flags, struct sockaddr *a, socklen_t *an);
int ib_setsockopt(int s, int level, int name, const void *v, socklen_t n);
int ib_getsockopt(int s, int level, int name, void *v, socklen_t *n);
int ib_getsockname(int s, struct sockaddr *a, socklen_t *n);
int ib_shutdown(int s, int how);
int ib_sendmsg(int s, const struct msghdr *m, int flags);
int ib_recvmsg(int s, struct msghdr *m, int flags);
int ib_select(int n, fd_set *r, fd_set *w, fd_set *e, struct timeval *t);
int ib_ioctl(int s, unsigned long req, ...);
int ib_close(int s);
int ib_fcntl(int s, int cmd, ...);
struct hostent *ib_gethostbyname(const char *name);
struct hostent *ib_gethostbyaddr(const void *addr, socklen_t len, int type);
int ib_inet_aton(const char *cp, struct in_addr *a);
char *ib_inet_ntoa(struct in_addr a);
unsigned long ib_inet_addr(const char *cp);
int ib_getaddrinfo(const char *host, const char *port, const struct addrinfo *hints, struct addrinfo **res);
void ib_freeaddrinfo(struct addrinfo *ai);
const char *ib_gai_strerror(int err);
const char *ib_inet_ntop(int af, const void *src, char *dst, socklen_t size);
int ib_inet_pton(int af, const char *src, void *dst);

#endif
