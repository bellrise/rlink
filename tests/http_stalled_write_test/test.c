#include "../_shared/http_test.h"

int main(void)
{
        struct server_stream server;
        struct rlink_http http;

        server_stream_init(&server, "", 0);
        server.stall_writes = 1;

        CHECK_INT(rlink_http(&http, &server.link, RL_HTTP_1_1), 0);
        CHECK_INT(rlink_http_putver(&http, "GET", "/health"), EPIPE);
        CHECK_STR(server.output, "");

        return test_done("http_stalled_write_test");
}
