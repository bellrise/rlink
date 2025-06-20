/* rlink vec routines
   Copyright (c) 2025 bellrise */

#include <rlink/rlink.h>
#include <stdlib.h>

int rlink_vec_put(struct rlink_vec *vec, void *p)
{
        vec->elems = realloc(vec->elems, sizeof(void *) * (vec->size + 1));
        vec->elems[vec->size++] = p;
        return 0;
}
