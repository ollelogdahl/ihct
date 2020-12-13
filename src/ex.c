#include "ihct.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BOOLSTR(s) s ? "true" : "false"

const unsigned ihct_test_count = 4;
unsigned ihct_testi = 0;
void (*ihct_tests_fn[4])(struct test_result*);
// An array of ptrs to the names of every test.
char **ihct_tests_name;
// An array of all first failed (or last if all successful) assert results in every test.
struct test_result **ihct_results;<

IHCT_TEST(arithmetic_addition_basic) {
    IHCT_ASSERT(1 + 2 == 3);
    IHCT_ASSERT(4 + 2 == 6);
    IHCT_ASSERT(8 + 8 == 16);
}
IHCT_TEST(arithmetic_addition_big) {
    IHCT_ASSERT(1000 + 2000 == 3000);
    IHCT_ASSERT(4000 + 2000 == 6000);
    IHCT_ASSERT(300 * 1231 == 3);
    IHCT_ASSERT(8000 + 8000 == 16000);
}
IHCT_TEST(arithmetic_multiplication) {
    IHCT_ASSERT(1 * 3 == 3);
    IHCT_ASSERT(0 * 3 == 0);
    IHCT_ASSERT(3 * 3 == 9);
}

IHCT_TEST(strings_basic) {
    IHCT_ASSERT(!strcmp("abba", "abba"));
    IHCT_ASSERT(strcmp("abba", "Abba") == 0);
}

int main(int argc, char **argv) {
    // ALL SHOULD LATER BE MOVED TO IHCT_RUN ---

    /*

    // This should not be needed! Figure it out.
    ihct_results = calloc(ihct_test_count, sizeof(struct test_result*));
    ihct_tests_name = calloc(ihct_test_count, sizeof(struct test_result*));
    printf("test count: %d\n", ihct_test_count);

    ihct_tests_fn[0] = &test_arithmetic_addition_basic;
    ihct_tests_name[0] = calloc(26, sizeof(char));
    strcpy(ihct_tests_name[0], "arithmetic_addition_basic");

    ihct_tests_fn[1] = &test_arithmetic_addition_big;
    ihct_tests_name[1] = calloc(24, sizeof(char));
    strcpy(ihct_tests_name[1], "arithmetic_addition_big");

    ihct_tests_fn[2] = &test_arithmetic_multiplication;
    ihct_tests_name[2] = calloc(26, sizeof(char));
    strcpy(ihct_tests_name[2], "arithmetic_multiplication");

    ihct_tests_fn[3] = &test_strings_basic;
    ihct_tests_name[3] = calloc(24, sizeof(char));
    strcpy(ihct_tests_name[3], "strings_basic");

    unsigned failed_count = 0;

    // Execute every test and add.
    for(unsigned i = 0; i < ihct_test_count; i++) {
        // Allocate memory for the result of the test.
        struct test_result *mem = malloc(sizeof(struct test_result));
        ihct_results[i] = mem;

        // Run test, and save it's status into i.
        (*ihct_tests_fn[i])(ihct_results[i]);

        if(!ihct_results[i]->passed) {
            char *assertion_format = IHCT_BOLD "%s:%d: "
                IHCT_RESET "assertion in '"
                IHCT_BOLD "%s"
                IHCT_RESET "' failed:\n\t'"
                IHCT_FOREGROUND_YELLOW "%s"
                IHCT_RESET "'\n";
            printf(assertion_format, ihct_results[i]->file, ihct_results[i]->line, 
                ihct_tests_name[i], ihct_results[i]->code);
            //printf("%s:%d: ", ihct_results[i]->file, ihct_results[i]->line);
            failed_count++;
        }
    }

    printf("\n");
    if(failed_count) {
        char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
            IHCT_RESET "and "
            IHCT_FOREGROUND_RED "%d failed "
            IHCT_RESET "of "
            IHCT_FOREGROUND_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, ihct_test_count-failed_count, failed_count, ihct_test_count);
    } 
    else {
        char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
            IHCT_RESET "of "
            IHCT_FOREGROUND_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, ihct_test_count, ihct_test_count);
    }

    if(failed_count) {
        printf(IHCT_FOREGROUND_RED "FAILURE\n" IHCT_RESET);
        return 1;
    }
    printf(IHCT_FOREGROUND_GREEN "SUCCESS\n" IHCT_RESET);
    return 0;
    // ---

    */

    return IHCT_RUN(argc, argv);
}