#include "ihct.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>

// The point of which to restore to on a fatal signal.
jmp_buf restore_point;
void *last_signal_context;
struct sigaction recover_action;

char *summary_extra_str;

// Handle when a signal is thrown
static void ihct_recovery_proc(int sig, siginfo_t *siginfo, void *context) {
    last_signal_context = context;
    longjmp(restore_point, sig);
}

static void ihct_setup_recover_action() {
    recover_action.sa_sigaction = &ihct_recovery_proc;
    sigemptyset(&recover_action.sa_mask);
    recover_action.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &recover_action, NULL);
    sigaction(SIGTERM, &recover_action, NULL);
}

// A list of all units.
static ihct_vector *testunits;

// An array of all first failed (or last if all successful) assert results in every test.
static ihct_test_result **ihct_results;

bool ihct_assert_impl(bool eval, ihct_test_result *result, char *code, char *file, 
                      unsigned long line) {
    result->passed = eval;

    if(!eval) {
        result->file = file;
        result->line = line;
        result->code = code;
        return false;
    }
    return true;
}

void ihct_construct_test_impl(char *name, ihct_test_proc procedure) {
    ihct_unit *unit = ihct_init_unit(name, procedure);
    ihct_vector_add(testunits, unit);
}

ihct_unit *ihct_init_unit(char *name, ihct_test_proc procedure) {
    ihct_unit *unit = (ihct_unit *)malloc(sizeof(ihct_unit));
    char *strmem = malloc(strlen(name) + 1);
    strcpy(strmem, name);
    unit->name = strmem;
    unit->procedure = procedure;

    return unit;
}

void ihct_unit_free(ihct_unit *unit) {
    free(unit->name);
    free(unit);
}

void ihct_init(void) {
    // atm, only initializes the unit list. Is this neccessary?
    testunits = ihct_init_vector();
}

ihct_vector *ihct_init_vector() {
    ihct_vector *v = malloc(sizeof(ihct_vector));
    if(v == NULL) {
        printf("Couldn't allocate memory for vector.\n");
        exit(EXIT_FAILURE);
    }
    v->size = 0;
    v->data = NULL;
}
void ihct_vector_add(ihct_vector *v, void *obj) {
    if(v->size == 0) {
        // Allocate a single 
        v->data = malloc(sizeof(obj));
        if(v->data == NULL) {
            printf("Couldn't allocate memory for object.\n");
            exit(EXIT_FAILURE);
        }
        v->data[0] = obj;
    } else {
        void *p = realloc(v->data, (v->size + 1) * sizeof(obj));
        if(p == NULL) {
            printf("Couldn't allocate memory for object.\n");
            exit(EXIT_FAILURE);
        }
        v->data = p;
        v->data[v->size] = obj;
    }
    v->size++;
}
void *ihct_vector_get(ihct_vector *v, int index) {
    return v->data[index];
}
void ihct_free_vector(ihct_vector *v) {
    free(v->data);
    v->data = NULL;
    free(v);
}

ihct_test_result *ihct_run_specific(ihct_unit *unit) {
    // Allocate memory for the tests result, and set it to passed by default.
    ihct_test_result *result = malloc(sizeof(ihct_test_result));
    result->passed = true;

    // Create a signal handler.
    ihct_setup_recover_action();

    // Create a jump point, to be able to resute when encountering segfault.
    int status = setjmp(restore_point);
    if(status != 0) {
        char *msg_format = "unit '"
            IHCT_BOLD "%s"
            IHCT_RESET "' had to restore because of fatal signal ("
            IHCT_FOREGROUND_RED "%s"
            IHCT_RESET ")\n";
        size_t msg_size = snprintf(NULL, 0, msg_format, unit->name, strsignal(status)) + 1;
        char *msg = calloc(msg_size, sizeof(char));
        char *p = realloc(summary_extra_str, strlen(summary_extra_str) + msg_size);
        summary_extra_str = p;
        sprintf(msg, msg_format, unit->name, strsignal(status));
        strcat(summary_extra_str, msg);

        result->code = "";
        result->file = "";
        result->line = 0;
        result->passed = false;
        return result;
    }

    // Run test, and save it's result into i.
    (*unit->procedure)(result);

    return result;
}

int ihct_run(int argc, char **argv) {
    unsigned unit_count = testunits->size;
    // Allocate results
    ihct_results = calloc(unit_count, sizeof(ihct_test_result *));

    unsigned failed_count = 0;

    summary_extra_str = calloc(0, sizeof(char));

    // start clock
    clock_t time_pretests = clock();

    // Iterate over every test
    for(unsigned i = 0; i < unit_count; i++) {
        ihct_unit *unit = ihct_vector_get(testunits, i);

        ihct_results[i] = ihct_run_specific(unit);

        if(ihct_results[i]->passed) {
            printf(IHCT_BACKGROUND_GREEN IHCT_BOLD "." IHCT_RESET);
        } else {
            printf(IHCT_BACKGROUND_RED IHCT_BOLD "!" IHCT_RESET);
            failed_count++;
        }
    }
    printf("\n%s", (failed_count > 0) ? "\n" : "");

    printf(summary_extra_str);

    clock_t time_posttests = clock();
    double elapsed = (double)(time_posttests - time_pretests) / CLOCKS_PER_SEC;

    for(unsigned i = 0; i < unit_count; ++i) {
        ihct_unit *unit = ihct_vector_get(testunits, i);

        if(!ihct_results[i]->passed) {
            char *assertion_format = IHCT_BOLD "%s:%d: "
                IHCT_RESET "assertion in '"
                IHCT_BOLD "%s"
                IHCT_RESET "' "
                IHCT_FOREGROUND_RED "failed"
                IHCT_RESET ":\n\t'"
                IHCT_FOREGROUND_YELLOW "%s"
                IHCT_RESET "'\n";
            printf(assertion_format, ihct_results[i]->file, ihct_results[i]->line, 
                   unit->name, ihct_results[i]->code);
        }
    }

    free(ihct_results);
    ihct_free_vector(testunits);

    printf("\ntests took %.2f seconds\n", elapsed);
    if(failed_count) {
        char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
            IHCT_RESET "and "
            IHCT_FOREGROUND_RED "%d failed "
            IHCT_RESET "of "
            IHCT_FOREGROUND_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, unit_count-failed_count, failed_count, 
               unit_count);
        
        printf(IHCT_FOREGROUND_RED "FAILURE\n" IHCT_RESET);
        return 1;
    } 
    
    char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
        IHCT_RESET "of "
        IHCT_FOREGROUND_YELLOW "%d run"
        IHCT_RESET "\n";
    printf(status_format, unit_count, unit_count);

    printf(IHCT_FOREGROUND_GREEN "SUCCESS\n" IHCT_RESET);
    return 0;
}

// Lets the program run tests on itself. This is done with the compiler flag
// IHCT_TEST_SELF. Still requires an external main entrypoint.
#ifdef IHCT_TEST_SELF

// Create two internal test procedures
static void itest_true(ihct_test_result *result) {
    IHCT_ASSERT(true);
}
static void itest_false(ihct_test_result *result) {
    IHCT_ASSERT(false);
}


IHCT_TEST(self_unit_create) {
    ihct_unit *u = ihct_init_unit("internal_true", &itest_true);
    IHCT_ASSERT_STR(u->name, "internal_true");
    IHCT_ASSERT(&itest_true == u->procedure);
}

IHCT_TEST(self_unit_run) {
    ihct_unit *u1 = ihct_init_unit("internal_true", &itest_true);
    ihct_unit *u2 = ihct_init_unit("internal_false", &itest_false);
    ihct_test_result *res1 = ihct_run_specific(u1);
    ihct_test_result *res2 = ihct_run_specific(u2);
    IHCT_ASSERT(res1->passed);
    IHCT_NASSERT(res2->passed);
}

IHCT_TEST(self_vector_create) {
    ihct_vector *v = ihct_init_vector();

    IHCT_ASSERT(v != NULL);
    IHCT_ASSERT(v->data == NULL);
    IHCT_ASSERT(v->size == 0);

    ihct_free_vector(v);
}

IHCT_TEST(self_vector_all) {
    ihct_vector *v = ihct_init_vector();

    for(int i = 0; i < 1000; ++i) {
        int *t = malloc(sizeof(int));
        *t = i * 2;
        ihct_vector_add(v, t);
    }

    for(int i = 0; i < 1000; ++i) {
        int *t = ihct_vector_get(v, i);
        IHCT_ASSERT(i * 2 == *t);
        free(t);
    }

    ihct_free_vector(v);
}
#endif