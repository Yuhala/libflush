#include <check.h>

#include "libflush.h"


static struct libflush_session_t *session;

static void setup_session() {
    fail_unless(libflush_init(&session, NULL) == true);
    fail_unless(session != NULL);
}

static void teardown_session() {
    fail_unless(libflush_terminate(session) == true);
    session = NULL;
}

START_TEST(test_libflush_init) {
    // Test invalid args
    fail_unless(libflush_init(NULL, NULL) == false);
} END_TEST

START_TEST(test_libflush_terminate) {
    // Test invalid args
    fail_unless(libflush_terminate(NULL) == false);
} END_TEST

Suite* suite_libflush() {
    Suite *suite = suite_create("libflush");
    TCase *tcase = NULL;
    
    tcase = tcase_create("basic");
    tcase_add_checked_fixture(tcase, setup_session, teardown_session);
    tcase_add_test(tcase, test_libflush_init);
    tcase_add_test(tcase, test_libflush_terminate);
    suite_add_tcase(suite, tcase);

    return suite;
}
