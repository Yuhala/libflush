#include <check.h>

#include "libflush.h"
#include "timing.h"


static struct libflush_session_t *session;

static void setup_session() {
    fail_unless(libflush_init(&session, NULL) == true);
    fail_unless(session != NULL);
}

static void teardown_session() {
    fail_unless(libflush_terminate(session) == true);
    session = NULL;
}

START_TEST(test_get_timing) {
    libflush_get_timing(session);
} END_TEST

START_TEST(test_reset_timing) {
    libflush_reset_timing(session);
} END_TEST


Suite* suite_timing() {
    Suite *suite = suite_create("timing");
    TCase *tcase = NULL;
    
    tcase = tcase_create("basic");
    tcase_add_checked_fixture(tcase, setup_session, teardown_session);
    tcase_add_test(tcase, test_get_timing);
    tcase_add_test(tcase, test_reset_timing);
    suite_add_tcase(suite, tcase);

    return suite;
}
