/* Generic rlink functions
   Copyright (c) 2025 bellrise */

#include <rlink/rlink.h>

int rlink_close(struct rlink *self)
{
        int err;

        /* Of course, first we need to call our own close() method before
           closing the underlying layer. */

        if (self->rl_close && (err = self->rl_close(self)))
                return err;
        if (self->rl_under && (err = rlink_close(self->rl_under)))
                return err;

        return 0;
}

ssize_t rlink_read(struct rlink *self, void *buf, ssize_t size)
{
        return self->rl_stream_read(self, buf, size);
}

ssize_t rlink_write(struct rlink *self, void *buf, ssize_t size)
{
        return self->rl_stream_write(self, buf, size);
}

ssize_t rlink_write_exact(struct rlink *self, void *buf, ssize_t size)
{
        ssize_t original_size;
        ssize_t nsent;

        original_size = size;

        do {
                nsent = self->rl_stream_write(self, buf, size);
                if (nsent < 0)
                        return nsent;
                if (nsent == size)
                        return original_size;
                size -= nsent;
                buf += nsent;
        } while (nsent > 0);

        return original_size;
}
