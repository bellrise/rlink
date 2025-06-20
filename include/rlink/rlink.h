/* rlink/rlink.h - RLINK header
   Copyright (c) 2025 bellrise */

#ifndef RLINK_H
#define RLINK_H 1

#include <arpa/inet.h>
#include <stddef.h>
#include <unistd.h>

/* Transport layer */
#define RLT_TCP 401
#define RLT_UDP 402

/* Session layer */
#define RLT_TLS 501

/* Application layer */
#define RLT_HTTP 601

#define RLF_STREAM 1 /* Layer provides a stream-like interface. */
#define RLF_SECURE 2 /* Layer provides security. */
#define RLF_TFD    4 /* Layer provides transparent access to underlying FD. */

struct rlink
{
        int rl_type;
        int rl_flags;
        struct rlink *rl_under;
        ssize_t (*rl_stream_read)(struct rlink *, void *, size_t);
        ssize_t (*rl_stream_write)(struct rlink *, void *, size_t);
        int (*rl_close)(struct rlink *);
        int (*rl_fd)(struct rlink *);
};

#define rlink_of(LAYER)             ((LAYER)->_self)
#define rlink_has_flag(RLINK, FLAG) ((RLINK)->rl_flags & (FLAG))
#define rlink_as_tcp(RLINK)         ((struct rlink_tcp *) (RLINK))
#define rlink_as_tls(RLINK)         ((struct rlink_tls *) (RLINK))
#define rlink_as_http(RLINK)        ((struct rlink_http *) (RLINK))

#if defined(RLINK_DEBUG)
# define rlink_debug(...) printf(__VA_ARGS__)
#else
# define rlink_debug(...)
#endif

struct rlink_tcp
{
        struct rlink _self;
        struct sockaddr_in rt_addr;
        int rt_sock;
};

struct rlink_tls
{
        struct rlink _self;
        void *internal;
};

struct rlink_http
{
        struct rlink _self;
        char http_ver[16];
};

struct rlink_http_ver
{
        int status;
        int ver;
};

struct rlink_http_hdr
{
        char *hdr;
        char *val;
};

struct rlink_vec
{
        void **elems;
        int size;
};

#define RL_HTTP_1_1 0x11

/* Close this and all underlying connections. Frees any memory associated with
   the whole stack. */
int rlink_close(struct rlink *);

/* Generic read & write applied to the layer. */
ssize_t rlink_read(struct rlink *, void *buf, ssize_t size);
ssize_t rlink_write(struct rlink *, void *buf, ssize_t size);

/* Connect to a peer via TCP over IPv4. `addr` can be an IPv4 address, or a
   domain name that gets resolved to an A record. TCP provides STREAM+TFD */
int rlink_tcp(struct rlink_tcp *, const char *addr_or_domain, int port);

/* Create a client TLS connection. Underlying layer has to support STREAM+TFD,
   the TFD requirement might be dropped in the future. */
int rlink_tls_client(struct rlink_tls *, struct rlink *over_layer);

/* Setup a HTTP connection. Underlying layer has to support STREAM.
  'http_version' should be one of RL_HTTP_. */
int rlink_http(struct rlink_http *, struct rlink *over_layer, int http_version);

int rlink_http_putver(struct rlink_http *, const char *method,
                      const char *path);
int rlink_http_puthdr(struct rlink_http *, const char *hdr, const char *val);
int rlink_http_endhdr(struct rlink_http *);
int rlink_http_getver(struct rlink_http *self, struct rlink_http_ver *);
int rlink_http_freehdrs(struct rlink_vec *headers);
const char *rlink_http_findhdr(struct rlink_vec *headers, const char *hdr);

/* Get single header-value pair from stream. Note that hdr & val are actually a
   single malloc area, so just free(hdr). */
int rlink_http_gethdr(struct rlink_http *self, struct rlink_http_hdr *);

/* Get single header-value pairs from stream until they run out, storing them
   in the vector, which needs to be freed by hand. */
int rlink_http_recvhdrs(struct rlink_http *self, struct rlink_vec *);

/* Copy the IPv4 address into 'a' or resolve an A record from a fqdn. */
int rlink_ipv4_resolve(struct in_addr *a, const char *address);

int rlink_vec_put(struct rlink_vec *, void *p);

#endif /* RLINK_H */
