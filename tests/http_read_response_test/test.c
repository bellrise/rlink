#include "../_shared/http_test.h"

int main(void)
{
        struct server_stream server;
        struct rlink_http http;
        struct rlink_http_ver ver;
        struct rlink_vec headers;

        server_stream_init(&server,
                           "HTTP/1.1 204 No Content\r\n"
                           "Content-Length: 0\r\n"
                           "X-Trace: abc123\r\n"
                           "\r\n",
                           0);

        CHECK_INT(rlink_http(&http, &server.link, RL_HTTP_1_1), 0);
        CHECK_INT(rlink_http_getver(&http, &ver), 0);
        CHECK_INT(ver.ver, RL_HTTP_1_1);
        CHECK_INT(ver.status, 204);

        CHECK_INT(rlink_http_recvhdrs(&http, &headers), 0);
        CHECK_INT(headers.size, 2);
        CHECK_STR(rlink_http_findhdr(&headers, "content-length"), "0");
        CHECK_STR(rlink_http_findhdr(&headers, "x-trace"), "abc123");
        CHECK(rlink_http_findhdr(&headers, "missing") == NULL);
        rlink_http_freehdrs(&headers);

        return test_done("http_read_response_test");
}
