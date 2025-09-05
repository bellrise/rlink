/* rlink_ipv4_ functions
   Copyright (c) 2025 bellrise */

#include <netdb.h>
#include <rlink/rlink.h>
#include <string.h>

int rlink_ipv4_resolve(struct in_addr *a, const char *name)
{
        struct addrinfo *walker;
        struct addrinfo *res;
        struct addrinfo hints;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(name, NULL, &hints, &res))
                return 1;
        for (walker = res; walker; walker = walker->ai_next)
                *a = ((struct sockaddr_in *) walker->ai_addr)->sin_addr;

        freeaddrinfo(res);
        return 0;
}
