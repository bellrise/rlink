/* rlink vec routines
   Copyright (c) 2025 bellrise */

#include <errno.h>
#include <rlink/rlink.h>
#include <stdlib.h>

int rlink_vec_put(struct rlink_vec *vec, void *p)
{
        vec->elems = realloc(vec->elems, sizeof(void *) * (vec->size + 1));
        if (vec->elems == NULL)
                return ENOMEM;

        vec->elems[vec->size++] = p;
        return 0;
}
