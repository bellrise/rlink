/* rlink http layer
   Copyright (c) 2025 bellrise */

#include <ctype.h>
#include <errno.h>
#include <rlink/rlink.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int http_close(struct rlink __rl_unused *_self)
{
        return 0;
}

ssize_t http_read(struct rlink *_self, void *buf, size_t size)
{
        return rlink_read(_self->rl_under, buf, size);
}

ssize_t http_write(struct rlink *_self, void *buf, size_t size)
{
        return rlink_write(_self->rl_under, buf, size);
}

int rlink_http(struct rlink_http *self, struct rlink *over_layer,
               int http_version)
{
        if (!rlink_has_flag(over_layer, RLF_STREAM))
                return EPROTO;
        if (http_version != RL_HTTP_1_1)
                return EINVAL;

        memset(self, 0, sizeof(*self));
        snprintf(self->http_ver, 16, "HTTP/1.1");

        rlink_of(self).rl_under = over_layer;
        rlink_of(self).rl_close = http_close;
        rlink_of(self).rl_stream_read = http_read;
        rlink_of(self).rl_stream_write = http_write;

        return 0;
}

int lazy_snprintf(char **res, const char *fmt, ...)
{
        va_list args;
        int bufsiz;
        int siz;
        char *buf;

        va_start(args, fmt);
        bufsiz = 0;
        buf = NULL;

        do {
                bufsiz += 256;
                buf = realloc(buf, bufsiz);
                siz = vsnprintf(buf, bufsiz, fmt, args);
        } while (siz >= bufsiz);

        va_end(args);
        *res = buf;
        return siz;
}

int rlink_http_putver(struct rlink_http *self, const char *method,
                      const char *path)
{
        struct rlink *link = self->_self.rl_under;
        char *line;
        int len;

        len =
            lazy_snprintf(&line, "%s %s %s\r\n", method, path, self->http_ver);
        if (link->rl_stream_write(link, line, len) < len)
                return errno;

        free(line);
        return 0;
}

int rlink_http_puthdr(struct rlink_http *self, const char *hdr, const char *val)
{
        struct rlink *link = self->_self.rl_under;
        char *line;
        int len;

        len = lazy_snprintf(&line, "%s: %s\r\n", hdr, val);
        if (link->rl_stream_write(link, line, len) < len)
                return errno;

        free(line);
        return 0;
}

int rlink_http_endhdr(struct rlink_http *self)
{
        struct rlink *link = self->_self.rl_under;
        if (link->rl_stream_write(link, "\r\n", 2) < 2)
                return errno;
        return 0;
}

static int stream_rdline(struct rlink *link, char *line, int limit)
{
        for (int i = 0; i < limit; i++) {
                if (link->rl_stream_read(link, &line[i], 1) != 1)
                        return errno;
                if (i > 0 && line[i - 1] == '\r' && line[i] == '\n') {
                        line[i - 1] = 0;
                        break;
                }

                if (i == limit - 1)
                        return EOVERFLOW;
        }

        return 0;
}

int rlink_http_getver(struct rlink_http *self, struct rlink_http_ver *ver)
{
        struct rlink *link = self->_self.rl_under;
        char head[256];
        int err;

        if ((err = stream_rdline(link, head, 256)))
                return err;

        if (strncmp(head, "HTTP/1.1 ", 9))
                return EPROTO;

        ver->ver = RL_HTTP_1_1;
        ver->status = strtol(head + 9, NULL, 10);
        if (ver->status < 100 || ver->status > 599)
                return EPROTO;

        return 0;
}

char *lstrip(char *p)
{
        while (*p && isspace(*p))
                p++;
        return p;
}

int rlink_http_gethdr(struct rlink_http *self, struct rlink_http_hdr *hdr)
{
        struct rlink *link = self->_self.rl_under;
        char line[512];
        char *p;
        int err;

        if ((err = stream_rdline(link, line, 512)))
                return err;

        if (*line == 0) {
                hdr->hdr = NULL;
                return 0;
        }

        hdr->hdr = strdup(line);
        p = strchr(hdr->hdr, ':');
        if (!p)
                return EPROTO;

        *(p++) = 0;
        hdr->val = lstrip(p);

        return 0;
}

int rlink_http_recvhdrs(struct rlink_http *self, struct rlink_vec *vec)
{
        struct rlink_http_hdr *hdr_copy;
        struct rlink_http_hdr hdr;
        int err;

        memset(vec, 0, sizeof(*vec));

        while (1) {
                if ((err = rlink_http_gethdr(self, &hdr)))
                        return err;
                if (!hdr.hdr)
                        break;

                hdr_copy = malloc(sizeof(hdr));
                hdr_copy->hdr = hdr.hdr;
                hdr_copy->val = hdr.val;
                rlink_vec_put(vec, hdr_copy);
        }

        return 0;
}

int rlink_http_freehdrs(struct rlink_vec *headers)
{
        struct rlink_http_hdr *phdr;
        for (int i = 0; i < headers->size; i++) {
                phdr = headers->elems[i];
                free(phdr->hdr);
                free(phdr);
        }

        free(headers->elems);
        headers->elems = NULL;
        headers->size = 0;

        return 0;
}

const char *rlink_http_findhdr(struct rlink_vec *headers, const char *hdr)
{
        struct rlink_http_hdr *phdr;
        for (int i = 0; i < headers->size; i++) {
                phdr = headers->elems[i];
                if (!strcmp(phdr->hdr, hdr))
                        return phdr->val;
        }

        return NULL;
}
