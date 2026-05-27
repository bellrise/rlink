#include "../_shared/http_test.h"

int main(void)
{
        struct server_stream server;
        struct rlink_http http;
        struct rlink_http_ver ver;

        server_stream_init(&server, "HTTP/1.1 200 OK", 0);

        CHECK_INT(rlink_http(&http, &server.link, RL_HTTP_1_1), 0);
        CHECK_INT(rlink_http_getver(&http, &ver), ECONNRESET);

        return test_done("http_truncated_response_test");
}
