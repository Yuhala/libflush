#include <check.h>

#include "libflush.h"
#include "eviction.h"


static struct libflush_session_t *session;

static void setup_session() {
    fail_unless(libflush_init(&session, NULL) == true);
    fail_unless(session != NULL);
}

static void teardown_session() {
    fail_unless(libflush_terminate(session) == true);
    session = NULL;
}

START_TEST(test_libflush_eviction_init) {
    // Test invalid args
    fail_unless(libflush_eviction_init(NULL, NULL) == false);
} END_TEST

START_TEST(test_libflush_eviction_terminate) {
    // Test invalid args
    fail_unless(libflush_eviction_terminate(NULL) == false);
} END_TEST

START_TEST(test_eviction_evict) {
    int x;
    libflush_eviction_evict(session, &x);
} END_TEST

START_TEST(test_eviction_evict_cached) {
    int x;
    libflush_eviction_evict(session, &x);
    libflush_eviction_evict(session, &x);
} END_TEST

START_TEST(test_eviction_evict_two) {
    int x;
    int y;
    libflush_eviction_evict(session, &x);
    libflush_eviction_evict(session, &y);
} END_TEST

Suite* suite_eviction() {
    Suite *suite = suite_create("eviction");
    TCase *tcase = NULL;
    
    tcase = tcase_create("basic");
    tcase_add_checked_fixture(tcase, setup_session, teardown_session);
    tcase_add_test(tcase, test_libflush_eviction_init);
    tcase_add_test(tcase, test_libflush_eviction_terminate);
    tcase_add_test(tcase, test_eviction_evict);
    tcase_add_test(tcase, test_eviction_evict_cached);
    tcase_add_test(tcase, test_eviction_evict_two);
    suite_add_tcase(suite, tcase);

    return suite;
}
