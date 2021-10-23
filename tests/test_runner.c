#include <check.h>
#include <stdlib.h>
#include <stdio.h>


Suite* suite_libflush();
Suite* suite_eviction();
Suite* suite_memory();
Suite* suite_timing();

int main() {
    SRunner *suite_runner = srunner_create(NULL);
    srunner_add_suite(suite_runner, suite_libflush());
    srunner_add_suite(suite_runner, suite_eviction());
    srunner_add_suite(suite_runner, suite_memory());
    srunner_add_suite(suite_runner, suite_timing());
    
    int failed = 0;

    printf("Running tests \n");
    srunner_run_all(suite_runner, CK_ENV);
    failed += srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);

    return (failed == 0) ? 0 : 1;
}
