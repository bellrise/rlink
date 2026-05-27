/* Shared HTTP test helpers. */

#ifndef RLINK_HTTP_TEST_H
# define RLINK_HTTP_TEST_H 1

# include <errno.h>
# include <rlink/rlink.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# if defined(__GNUC__)
#  define TEST_UNUSED __attribute__((unused))
# else
#  define TEST_UNUSED
# endif

struct server_stream
{
        struct rlink link;
        const char *input;
        size_t input_len;
        size_t input_pos;
        char output[1024];
        size_t output_len;
        size_t max_write;
        int stall_writes;
};

static int failures;

# define CHECK(EXPR)                                                           \
         do {                                                                  \
                 if (!(EXPR)) {                                                \
                         fprintf(stderr, "%s:%d: check failed: %s\n",          \
                                 __FILE__, __LINE__, #EXPR);                   \
                         failures++;                                           \
                 }                                                             \
         } while (0)

static void TEST_UNUSED check_int(int got, int want, const char *expr, int line)
{
        if (got != want) {
                fprintf(stderr, "%s:%d: %s got %d, want %d\n", __FILE__, line,
                        expr, got, want);
                failures++;
        }
}

static void TEST_UNUSED check_str(const char *got, const char *want,
                                  const char *expr, int line)
{
        if (!got || strcmp(got, want) != 0) {
                fprintf(stderr, "%s:%d: %s got \"%s\", want \"%s\"\n", __FILE__,
                        line, expr, got ? got : "(null)", want);
                failures++;
        }
}

# define CHECK_INT(GOT, WANT) check_int((GOT), (WANT), #GOT, __LINE__)
# define CHECK_STR(GOT, WANT) check_str((GOT), (WANT), #GOT, __LINE__)

static ssize_t server_read(struct rlink *link, void *buf, size_t size)
{
        struct server_stream *server = (struct server_stream *) link;
        size_t available;

        if (server->input_pos >= server->input_len)
                return 0;

        available = server->input_len - server->input_pos;
        if (size > available)
                size = available;

        memcpy(buf, server->input + server->input_pos, size);
        server->input_pos += size;
        return (ssize_t) size;
}

static ssize_t server_write(struct rlink *link, void *buf, size_t size)
{
        struct server_stream *server = (struct server_stream *) link;

        if (server->stall_writes)
                return 0;

        if (server->max_write && size > server->max_write)
                size = server->max_write;

        if (server->output_len + size >= sizeof(server->output)) {
                errno = ENOSPC;
                return -1;
        }

        memcpy(server->output + server->output_len, buf, size);
        server->output_len += size;
        server->output[server->output_len] = 0;
        return (ssize_t) size;
}

static void server_stream_init(struct server_stream *server, const char *input,
                               size_t max_write)
{
        memset(server, 0, sizeof(*server));
        server->input = input;
        server->input_len = strlen(input);
        server->max_write = max_write;
        server->link.rl_flags = RLF_STREAM;
        server->link.rl_stream_read = server_read;
        server->link.rl_stream_write = server_write;
}

static int test_done(const char *name)
{
        if (failures) {
                fprintf(stderr, "%s: %d check(s) failed\n", name, failures);
                return 1;
        }

        return 0;
}

#endif /* RLINK_HTTP_TEST_H */
