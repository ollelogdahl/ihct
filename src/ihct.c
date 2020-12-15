#include "ihct.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>

// The point of which to restore to on a fatal signal.
jmp_buf restore_environment;
// the sigaction used for catching fatal signals.
struct sigaction recover_action;
// The final summary string printed.
char *summary_str;
// A list of all units.
static ihct_vector *testunits;
// An array of all first failed (or last if all successful) assert results in every test.
static ihct_test_result **ihct_results;

// Procedure called when a signal is thrown within a test. When a fatal signal is
// recieved, we jump back to before the test is ran, giving the signal code.
static void ihct_recovery_proc(int sig, siginfo_t *siginfo, void *context) {
    longjmp(restore_environment, sig);
}

// Binds the sigaction to signals, and make it call ihct_recovery_proc.
static void ihct_setup_recover_action() {
    recover_action.sa_sigaction = &ihct_recovery_proc;
    sigemptyset(&recover_action.sa_mask);
    recover_action.sa_flags = SA_SIGINFO;
    // binding sigactions
    sigaction(SIGSEGV, &recover_action, NULL);
    sigaction(SIGTERM, &recover_action, NULL);
}


void ihct_print_result(ihct_test_result *result) {
    switch (result->status) {
    case FAIL: printf(IHCT_BG_RED IHCT_BOLD ":" IHCT_RESET); break;
    case PASS: printf(IHCT_BG_GREEN IHCT_BOLD "." IHCT_RESET); break;
    case ERR: printf(IHCT_BG_RED IHCT_BOLD "!" IHCT_RESET); break;
    }
}
// Reallocates and appends string s to summary_str
void ihct_add_to_summary(char *s) {
    char *p = realloc(summary_str, strlen(summary_str) + strlen(s));
    summary_str = p;
    strcat(summary_str, s);
}
void ihct_add_error_to_summary(ihct_test_result *res, ihct_unit *unit) {
    char *assertion_format = IHCT_BOLD "%s:%d: "
        IHCT_RESET "assertion in '"
        IHCT_BOLD "%s"
        IHCT_RESET "' "
        IHCT_FG_RED "failed"
        IHCT_RESET ":\n\t'"
        IHCT_FG_YELLOW "%s"
        IHCT_RESET "'\n";
    size_t msg_size = snprintf(NULL, 0, assertion_format, res->file, res->line, 
           unit->name, res->code) + 1;
    char *msg = calloc(msg_size, sizeof(char));
    sprintf(msg, assertion_format, res->file, res->line, unit->name, res->code);
    ihct_add_to_summary(msg);
}

bool ihct_assert_impl(bool eval, ihct_test_result *result, char *code, char *file, 
                      unsigned long line) {
    result->status = eval;

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
    result->status = true;

    // Create a signal handler.
    ihct_setup_recover_action();

    // Create a jump point, to be able to resute when encountering segfault.
    int status = setjmp(restore_environment);
    if(status != 0) {
        char *msg_format = "unit '"
            IHCT_BOLD "%s"
            IHCT_RESET "' had to restore because of fatal signal ("
            IHCT_FG_RED "%s"
            IHCT_RESET ")\n";
        size_t msg_size = snprintf(NULL, 0, msg_format, unit->name, strsignal(status)) + 1;
        char *msg = calloc(msg_size, sizeof(char));
        sprintf(msg, msg_format, unit->name, strsignal(status));
        ihct_add_to_summary(msg);

        // Create an empty result
        result->code = "";
        result->file = "";
        result->line = 0;
        result->status = ERR;
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

    // initialize the summary string
    summary_str = calloc(0, sizeof(char));

    // start clock
    clock_t time_pretests = clock();

    // Iterate over every test
    for(unsigned i = 0; i < unit_count; i++) {
        ihct_unit *unit = ihct_vector_get(testunits, i);

        ihct_results[i] = ihct_run_specific(unit);

        ihct_print_result(ihct_results[i]);

        if(ihct_results[i]->status != PASS) {
            failed_count++;
        }
        if(ihct_results[i]->status == FAIL) {
            ihct_add_error_to_summary(ihct_results[i], unit);
        }
    }
    clock_t time_posttests = clock();
    double elapsed = (double)(time_posttests - time_pretests) / CLOCKS_PER_SEC;

    // print status
    printf("\n%s%s", (failed_count > 0) ? "\n" : "", summary_str);

    free(ihct_results);
    ihct_free_vector(testunits);

    printf("\ntests took %.2f seconds\n", elapsed);
    if(failed_count) {
        char *status_format = IHCT_FG_GREEN "%d successful "
            IHCT_RESET "and "
            IHCT_FG_RED "%d failed "
            IHCT_RESET "of "
            IHCT_FG_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, unit_count-failed_count, failed_count, 
               unit_count);
        
        printf(IHCT_FG_RED "FAILURE\n" IHCT_RESET);
        return 1;
    } 
    
    char *status_format = IHCT_FG_GREEN "%d successful "
        IHCT_RESET "of "
        IHCT_FG_YELLOW "%d run"
        IHCT_RESET "\n";
    printf(status_format, unit_count, unit_count);

    printf(IHCT_FG_GREEN "SUCCESS\n" IHCT_RESET);
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
    IHCT_ASSERT(res1->status == 1);
    IHCT_NASSERT(res2->status == 1);
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