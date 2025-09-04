/* rlink stream layer
   Copyright (c) 2025 bellrise */

#include <netdb.h>
#include <rlink/rlink.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

static ssize_t stream_read(struct rlink *_self, void *buf, size_t size)
{
        return read(rlink_as_sock_stream(_self)->rs_sock, buf, size);
}

static ssize_t stream_write(struct rlink *_self, void *buf, size_t size)
{
        return write(rlink_as_sock_stream(_self)->rs_sock, buf, size);
}

static int stream_close(struct rlink *_self)
{
        if (!rlink_as_sock_stream(_self)->rs_autoclose)
                return 0;
        return close(rlink_as_sock_stream(_self)->rs_sock);
}

static int stream_fd(struct rlink *_self)
{
        return rlink_as_sock_stream(_self)->rs_sock;
}

int rlink_sock_stream(struct rlink_sock_stream *self, int stream_sock,
                      int auto_close)
{
        struct rlink *_self;

        memset(self, 0, sizeof(*self));

        self->rs_sock = stream_sock;
        self->rs_autoclose = auto_close;

        /* Define the layer */

        _self = &rlink_of(self);
        _self->rl_flags = RLF_STREAM | RLF_TFD;
        _self->rl_stream_write = stream_write;
        _self->rl_stream_read = stream_read;
        _self->rl_close = stream_close;
        _self->rl_fd = stream_fd;
        _self->rl_under = NULL;
        _self->rl_type = RLT_SOCK_STREAM;

        return 0;
}
