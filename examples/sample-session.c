#include <stdio.h>
#include <stdlib.h>

#include "eviction.h"
#include "libflush.h"
#include "internal.h"


int main() {

    struct libflush_session_t **lst;
    lst = malloc(sizeof(struct libflush_session_t **));
    struct libflush_session_args_t *sess_args;
    sess_args = malloc(sizeof(struct libflush_session_args_t));
    libflush_init(lst, sess_args);
    struct libflush_session_t *ls = *lst;
    libflush_terminate(ls);

    return 0;
}
