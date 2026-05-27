#include "../_shared/http_test.h"

int main(void)
{
        struct server_stream server;
        struct rlink_http http;
        struct rlink_http_hdr hdr;

        server_stream_init(&server, "BrokenHeader\r\n", 0);
        hdr.hdr = (char *) 1;
        hdr.val = (char *) 1;

        CHECK_INT(rlink_http(&http, &server.link, RL_HTTP_1_1), 0);
        CHECK_INT(rlink_http_gethdr(&http, &hdr), EPROTO);
        CHECK(hdr.hdr == NULL);

        return test_done("http_malformed_header_test");
}
