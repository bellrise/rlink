#include "../_shared/http_test.h"

int main(void)
{
        struct server_stream server;
        struct rlink_http http;

        server_stream_init(&server, "", 3);

        CHECK_INT(rlink_http(&http, &server.link, RL_HTTP_1_1), 0);
        CHECK_INT(rlink_http_putver(&http, "GET", "/health"), 0);
        CHECK_INT(rlink_http_puthdr(&http, "Host", "example.test"), 0);
        CHECK_INT(rlink_http_puthdr(&http, "Accept", "*/*"), 0);
        CHECK_INT(rlink_http_endhdr(&http), 0);
        CHECK_STR(server.output, "GET /health HTTP/1.1\r\n"
                                 "Host: example.test\r\n"
                                 "Accept: */*\r\n"
                                 "\r\n");

        return test_done("http_write_request_test");
}
