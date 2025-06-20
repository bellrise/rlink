#include <rlink/rlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
        struct rlink_tcp transport;
        struct rlink_http server;
        struct rlink_tls session;
        char *buf;
        int siz;

        if (argc < 2) {
                printf("usage: ./tls-curl <host>\n");
                return 1;
        }

        rlink_tcp(&transport, argv[1], 443);
        rlink_tls_client(&session, (struct rlink *) &transport);
        rlink_http(&server, (struct rlink *) &session, RL_HTTP_1_1);

        // Send a request.

        rlink_http_putver(&server, "GET", "/");
        rlink_http_puthdr(&server, "Content-Length", "0");
        rlink_http_puthdr(&server, "Host", argv[1]);
        rlink_http_endhdr(&server);

        struct rlink_http_ver ver;
        struct rlink_http_hdr *hdr;
        struct rlink_vec hdrs;

        rlink_http_getver(&server, &ver);
        rlink_http_recvhdrs(&server, &hdrs);

#if 0
        printf("HTTP/1.1 %d\n", ver.status);


        for (int i = 0; i < hdrs.size; i++) {
                hdr = hdrs.elems[i];
                printf("\033[1m%s:\033[m %s\n", hdr->hdr, hdr->val);
        }

        printf("\n");
#endif

#if 1
        // Just dump everything...

        siz = strtol(rlink_http_findhdr(&hdrs, "Content-Length"), NULL, 10);
        buf = malloc(siz);
        rlink_read((struct rlink *) &server, buf, siz);
        printf("%.*s", siz, buf);
        free(buf);

#endif
        rlink_http_freehdrs(&hdrs);
        rlink_close((struct rlink *) &server);
}
