/* Iceball on GFOS: the program's start, and the socket calls of
 * ib_bsdsocket.h on bsdsocket.library.
 *
 * main() opens bsdsocket.library when a TCP/IP stack is running, then runs
 * Iceball's own main (src/main.c, built as ib_iceball_main). Started with no
 * arguments it shows Iceball's launcher (local game, server list, settings)
 * when there is a stack, or goes straight into a local game on the base mod
 * when there is none, since the launcher's other choices need the network.
 * GPL-3.0-or-later, like Iceball. Copyright (c) 2026 AmiCradle Team. */
#define IB_AMIGA_C 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include "ib_bsdsocket.h"
#include "ib_net_names.h"

struct Library *SocketBase;

/* Lua, the map code and the mods recurse deeply */
unsigned long __stack = 512 * 1024;
static const char version[] __attribute__((used)) = "$VER: Iceball 0.2.1 (16.11.2026) GFOS";

int ib_iceball_main(int argc, char *argv[]);

static void close_socket_library(void)
{
	if (SocketBase) {
		CloseLibrary(SocketBase);
		SocketBase = NULL;
	}
}

static void open_socket_library(void)
{
	SocketBase = OpenLibrary((CONST_STRPTR)"bsdsocket.library", 3);
	if (!SocketBase)
		return;
	atexit(close_socket_library);
	/* the stack's error codes into the C library's errno (version 4 on) */
	if (SocketBase->lib_Version >= 4)
		SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), (ULONG)&errno, TAG_END);
}

int main(int argc, char *argv[])
{
	static char *local[] = { "Iceball", "-s", "0", "pkg/base", NULL };
	BPTR old_dir;
	int rc;

	/* pkg/, clsave/ and svsave/ sit beside the program */
	old_dir = CurrentDir(GetProgramDir());
	open_socket_library();
	if (argc <= 1 && !SocketBase) {
		puts("Iceball: no TCP/IP stack running, so a local game on the base mod.");
		rc = ib_iceball_main(4, local);
	} else {
		rc = ib_iceball_main(argc, argv);
	}
	CurrentDir(old_dir);
	return rc;
}

/* ---- the socket calls ---- */

#define NO_STACK(ret) do { if (!SocketBase) { errno = ENETDOWN; return ret; } } while (0)

int ib_socket(int domain, int type, int protocol)
{
	NO_STACK(-1);
	if (domain != AF_INET) {
		errno = EAFNOSUPPORT;
		return -1;
	}
	return socket(domain, type, protocol);
}

int ib_connect(int s, const struct sockaddr *a, socklen_t n)
{
	NO_STACK(-1);
	return connect(s, (struct sockaddr *)a, n);
}

int ib_bind(int s, const struct sockaddr *a, socklen_t n)
{
	NO_STACK(-1);
	return bind(s, (struct sockaddr *)a, n);
}

int ib_listen(int s, int backlog)
{
	NO_STACK(-1);
	return listen(s, backlog);
}

int ib_accept(int s, struct sockaddr *a, socklen_t *n)
{
	NO_STACK(-1);
	return accept(s, a, n);
}

int ib_send(int s, const void *b, size_t n, int flags)
{
	NO_STACK(-1);
	return send(s, (APTR)b, n, flags);
}

int ib_recv(int s, void *b, size_t n, int flags)
{
	NO_STACK(-1);
	return recv(s, b, n, flags);
}

int ib_sendto(int s, const void *b, size_t n, int flags, const struct sockaddr *a, socklen_t an)
{
	NO_STACK(-1);
	return sendto(s, (APTR)b, n, flags, (struct sockaddr *)a, an);
}

int ib_recvfrom(int s, void *b, size_t n, int flags, struct sockaddr *a, socklen_t *an)
{
	NO_STACK(-1);
	return recvfrom(s, b, n, flags, a, an);
}

int ib_setsockopt(int s, int level, int name, const void *v, socklen_t n)
{
	NO_STACK(-1);
	return setsockopt(s, level, name, (APTR)v, n);
}

int ib_getsockopt(int s, int level, int name, void *v, socklen_t *n)
{
	NO_STACK(-1);
	return getsockopt(s, level, name, v, n);
}

int ib_getsockname(int s, struct sockaddr *a, socklen_t *n)
{
	NO_STACK(-1);
	return getsockname(s, a, n);
}

int ib_shutdown(int s, int how)
{
	NO_STACK(-1);
	return shutdown(s, how);
}

int ib_sendmsg(int s, const struct msghdr *m, int flags)
{
	NO_STACK(-1);
	return sendmsg(s, (struct msghdr *)m, flags);
}

int ib_recvmsg(int s, struct msghdr *m, int flags)
{
	NO_STACK(-1);
	return recvmsg(s, m, flags);
}

int ib_select(int n, fd_set *r, fd_set *w, fd_set *e, struct timeval *t)
{
	NO_STACK(-1);
	return WaitSelect(n, r, w, e, t, NULL);
}

int ib_ioctl(int s, unsigned long req, ...)
{
	va_list ap;
	char *arg;
	NO_STACK(-1);
	va_start(ap, req);
	arg = va_arg(ap, char *);
	va_end(ap);
	return IoctlSocket(s, req, arg);
}

int ib_close(int s)
{
	NO_STACK(-1);
	return CloseSocket(s);
}

/* only what Iceball and ENet ask: the non-blocking flag */
int ib_fcntl(int s, int cmd, ...)
{
	va_list ap;
	long flags, on;
	NO_STACK(-1);
	if (cmd == F_GETFL)
		return 0;
	if (cmd != F_SETFL) {
		errno = EINVAL;
		return -1;
	}
	va_start(ap, cmd);
	flags = va_arg(ap, long);
	va_end(ap);
	on = (flags & O_NONBLOCK) ? 1 : 0;
	return IoctlSocket(s, FIONBIO, (char *)&on);
}

struct hostent *ib_gethostbyname(const char *name)
{
	NO_STACK(NULL);
	return gethostbyname((STRPTR)name);
}

struct hostent *ib_gethostbyaddr(const void *addr, socklen_t len, int type)
{
	NO_STACK(NULL);
	return gethostbyaddr((STRPTR)addr, len, type);
}

/* ---- names and numbers: no library needed (ib_net_names.h) ---- */

int ib_inet_aton(const char *cp, struct in_addr *a)
{
	unsigned long v;
	if (!ib_parse_ipv4(cp, &v))
		return 0;
	a->s_addr = v;
	return 1;
}

unsigned long ib_inet_addr(const char *cp)
{
	unsigned long v;
	return ib_parse_ipv4(cp, &v) ? v : 0xFFFFFFFFUL;
}

char *ib_inet_ntoa(struct in_addr a)
{
	static char buf[16];
	ib_format_ipv4(a.s_addr, buf);
	return buf;
}

int ib_inet_pton(int af, const char *src, void *dst)
{
	unsigned long v;
	if (af != AF_INET) {
		errno = EAFNOSUPPORT;
		return -1;
	}
	if (!ib_parse_ipv4(src, &v))
		return 0;
	memcpy(dst, &v, 4);
	return 1;
}

const char *ib_inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	unsigned long v;
	char buf[16];
	if (af != AF_INET) {
		errno = EAFNOSUPPORT;
		return NULL;
	}
	memcpy(&v, src, 4);
	ib_format_ipv4(v, buf);
	if (strlen(buf) + 1 > (size_t)size) {
		errno = ENOSPC;
		return NULL;
	}
	strcpy(dst, buf);
	return dst;
}

const struct in6_addr ib_in6addr_any;

int ib_getaddrinfo(const char *host, const char *port, const struct addrinfo *hints, struct addrinfo **res)
{
	unsigned long ip;
	struct addrinfo *ai;
	struct sockaddr_in *sa;
	int family = hints ? hints->ai_family : AF_UNSPEC;

	*res = NULL;
	if (family != AF_UNSPEC && family != AF_INET)
		return EAI_NONAME;
	if (host == NULL) {
		ip = (hints && (hints->ai_flags & AI_PASSIVE)) ? INADDR_ANY : htonl(INADDR_LOOPBACK);
	} else if (!ib_parse_ipv4(host, &ip)) {
		struct hostent *he = ib_gethostbyname(host);
		if (he == NULL || he->h_addrtype != AF_INET || he->h_addr_list[0] == NULL)
			return EAI_NONAME;
		memcpy(&ip, he->h_addr_list[0], 4);
	}
	ai = calloc(1, sizeof(*ai) + sizeof(*sa));
	if (ai == NULL)
		return EAI_MEMORY;
	sa = (struct sockaddr_in *)(ai + 1);
	sa->sin_len = sizeof(*sa);
	sa->sin_family = AF_INET;
	sa->sin_port = htons((unsigned short)(port ? atoi(port) : 0));
	sa->sin_addr.s_addr = ip;
	ai->ai_family = AF_INET;
	ai->ai_socktype = hints ? hints->ai_socktype : 0;
	ai->ai_protocol = hints ? hints->ai_protocol : 0;
	ai->ai_addrlen = sizeof(*sa);
	ai->ai_addr = (struct sockaddr *)sa;
	*res = ai;
	return 0;
}

void ib_freeaddrinfo(struct addrinfo *ai)
{
	while (ai) {
		struct addrinfo *next = ai->ai_next;
		free(ai);
		ai = next;
	}
}

const char *ib_gai_strerror(int err)
{
	switch (err) {
	case EAI_NONAME: return "host not found";
	case EAI_MEMORY: return "out of memory";
	default: return "name lookup failed";
	}
}
