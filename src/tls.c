/* rlink TLS support
   Copyright (c) 2025 bellrise */

#include <errno.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <rlink/rlink.h>
#include <stdio.h>
#include <string.h>

struct tls_internal
{
        SSL_CTX *ssl_context;
        SSL *ssl;
};

#define INTERNAL(_SELF)                                                        \
        ((struct tls_internal *) (rlink_as_tls(_SELF))->internal)

static ssize_t tls_read(struct rlink *_self, void *buf, size_t size)
{
        return SSL_read(INTERNAL(_self)->ssl, buf, size);
}

static ssize_t tls_write(struct rlink *_self, void *buf, size_t size)
{
        return SSL_write(INTERNAL(_self)->ssl, buf, size);
}

static int tls_open(struct rlink *_self)
{
        SSL_CTX *ssl_context;
        SSL *ssl;
        int err;

        ssl_context = SSL_CTX_new(TLS_client_method());
        if (!ssl_context)
                return EPERM;

        SSL_CTX_set_min_proto_version(ssl_context, TLS1_2_VERSION);

        ssl = SSL_new(ssl_context);
        if (!ssl)
                return EPERM;

        if (!SSL_set_fd(ssl, _self->rl_under->rl_fd(_self->rl_under)))
                return EBADFD;

        err = SSL_connect(ssl);
        if (err != 1) {
                rlink_debug("rlink-ssl: %s\n", ERR_error_string(err, NULL));
                return EBADE;
        }

        INTERNAL(_self)->ssl_context = ssl_context;
        INTERNAL(_self)->ssl = ssl;

        return 0;
}

static int tls_close(struct rlink *_self)
{
        SSL_shutdown(INTERNAL(_self)->ssl);
        SSL_free(INTERNAL(_self)->ssl);
        SSL_CTX_free(INTERNAL(_self)->ssl_context);
        free(INTERNAL(_self));
        return 0;
}

int rlink_tls_client(struct rlink_tls *self, struct rlink *over_layer)
{
        struct rlink *_self;
        int err;

        if (!rlink_has_flag(over_layer, RLF_STREAM | RLF_TFD))
                return EINVAL;

        memset(self, 0, sizeof(*self));

        /* Define the layer */

        _self = &rlink_of(self);
        _self->rl_flags = RLF_STREAM;
        _self->rl_stream_read = tls_read;
        _self->rl_stream_write = tls_write;
        _self->rl_close = tls_close;
        _self->rl_type = RLT_TLS;
        _self->rl_under = over_layer;

        self->internal = malloc(sizeof(struct tls_internal));
        if ((err = tls_open(&rlink_of(self))))
                return err;

        return 0;
}
