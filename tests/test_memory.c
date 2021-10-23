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

START_TEST(test_flush) {
    int x;
    libflush_flush(session, &x);
} END_TEST

START_TEST(test_flush_time) {
    int x;
    libflush_flush_time(session, &x);
} END_TEST

START_TEST(test_evict_time) {
    int x;
    libflush_evict_time(session, &x);
} END_TEST

START_TEST(test_access_memory) {
    int x;
    libflush_access_memory(&x);
} END_TEST

START_TEST(test_reload_address) {
    int x;
    libflush_reload_addr(session, &x);
} END_TEST

START_TEST(test_reload_address_and_flush) {
    int x;
    libflush_reload_addr_and_flush(session, &x);
} END_TEST

START_TEST(test_memory_barrier) {
    libflush_memory_barrier();
} END_TEST



Suite* suite_memory() {
    Suite *suite = suite_create("memory");
    TCase *tcase = NULL;
    
    tcase = tcase_create("basic");
    tcase_add_checked_fixture(tcase, setup_session, teardown_session);
    tcase_add_test(tcase, test_flush);
    tcase_add_test(tcase, test_flush_time);
    tcase_add_test(tcase, test_evict_time);
    tcase_add_test(tcase, test_access_memory);
    tcase_add_test(tcase, test_reload_address);
    tcase_add_test(tcase, test_reload_address_and_flush);
    tcase_add_test(tcase, test_memory_barrier);
    suite_add_tcase(suite, tcase);

    return suite;
}
