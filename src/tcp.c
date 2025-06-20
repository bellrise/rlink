/* rlink tcp layer
   Copyright (c) 2025 bellrise */

#include <errno.h>
#include <netdb.h>
#include <rlink/rlink.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

static ssize_t tcp_read(struct rlink *_self, void *buf, size_t size)
{
        return read(rlink_as_tcp(_self)->rt_sock, buf, size);
}

static ssize_t tcp_write(struct rlink *_self, void *buf, size_t size)
{
        return write(rlink_as_tcp(_self)->rt_sock, buf, size);
}

static int tcp_close(struct rlink *_self)
{
        return close(rlink_as_tcp(_self)->rt_sock);
}

static int tcp_fd(struct rlink *_self)
{
        return rlink_as_tcp(_self)->rt_sock;
}

int rlink_tcp(struct rlink_tcp *self, const char *addr_or_domain, int port)
{
        struct rlink *_self;

        memset(self, 0, sizeof(*self));

        self->rt_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (self->rt_sock == -1)
                return errno;

        memset(&self->rt_addr, 0, sizeof(self->rt_addr));
        if (rlink_ipv4_resolve(&self->rt_addr.sin_addr, addr_or_domain))
                return EINVAL;
        self->rt_addr.sin_port = htons(port);
        self->rt_addr.sin_family = AF_INET;

        if (connect(self->rt_sock, (struct sockaddr *) &self->rt_addr,
                    sizeof(self->rt_addr))) {
                return errno;
        }

        rlink_debug("rlink-tcp: connected to %s:%d\n", addr_or_domain, port);

        /* Define the layer */

        _self = &rlink_of(self);
        _self->rl_flags = RLF_STREAM | RLF_TFD;
        _self->rl_stream_write = tcp_write;
        _self->rl_stream_read = tcp_read;
        _self->rl_close = tcp_close;
        _self->rl_fd = tcp_fd;
        _self->rl_under = NULL;
        _self->rl_type = RLT_TCP;

        return 0;
}
