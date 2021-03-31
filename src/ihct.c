#include "ihct.h"
#include "vector.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h> // handle tests that yield fatal signals.
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

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

// Object representing a testing unit, containing the units name and its procedure
// (implemented test function).
typedef struct {
    char *name;
    ihct_test_proc procedure;
} ihct_unit;


// The number of seconds passed until a test is considered timedout.
// Default 3. Can be set with -t [time in sec]
int test_timeout = 3;

pthread_cond_t routine_done = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// These are ISO/IEC 6429 escape sequences for
// communicating text attributes to terminal emulators.
// Note that some compilers do not understand '\x1b', and therefore \033[0m is 
// used instead.
#define IHCT_RESET "\033[0m"
#define IHCT_BOLD "\033[1m"
#define IHCT_FG_GRAY "\033[30;1m"
#define IHCT_FG_RED "\033[31;1m"
#define IHCT_FG_GREEN "\033[32;1m"
#define IHCT_FG_YELLOW "\033[33;1m"
#define IHCT_FG_BLUE "\033[34;1m"
#define IHCT_FG_MAGENTA "\033[35;1m"
#define IHCT_FG_CYAN "\033[36;1m"
#define IHCT_FG_WHITE "\033[37;1m"
#define IHCT_BG_BLACK "\033[40;1m"
#define IHCT_BG_RED "\033[41;1m"
#define IHCT_BG_GREEN "\033[42;1m"
#define IHCT_BG_YELLOW "\033[43;1m"
#define IHCT_BG_BLUE "\033[44;1m"
#define IHCT_BG_MAGENTA "\033[45;1m"
#define IHCT_BG_CYAN "\033[46;1m"
#define IHCT_BG_GRAY "\033[47;1m"

// Procedure called when a signal is thrown within a test. When a fatal signal is
// recieved, we jump back to before the test is ran, giving the signal code.
static void ihct_recovery_proc(int sig) {
    // Restore.
    longjmp(restore_environment, sig);
}

// Binds the sigaction to signals, and make it call ihct_recovery_proc.
static void ihct_setup_recover_action(void) {
    recover_action.sa_handler = &ihct_recovery_proc;
    sigemptyset(&recover_action.sa_mask);
    recover_action.sa_flags = 0;
    // binding sigactions. Dont pick up on user interrupts (let them be managed
    // normally).
    sigaction(SIGSEGV, &recover_action, NULL);
    sigaction(SIGTERM, &recover_action, NULL);
    sigaction(SIGFPE, &recover_action, NULL);
    sigaction(SIGILL, &recover_action, NULL);
    sigaction(SIGABRT, &recover_action, NULL);
    sigaction(SIGBUS, &recover_action, NULL);
}


void ihct_print_result(ihct_test_result *result) {
    switch (result->status) {
    case PASS: fputs(IHCT_BG_GREEN IHCT_BOLD "." IHCT_RESET, stdout); break;
    case FAIL_FORCE:
    case FAIL: fputs(IHCT_BG_RED IHCT_BOLD ":" IHCT_RESET, stdout); break;
    case ERR: fputs(IHCT_BG_RED IHCT_BOLD "!" IHCT_RESET, stdout); break;
    case TIMEOUT: fputs(IHCT_BG_YELLOW IHCT_BOLD "?" IHCT_RESET, stdout); break;
    }
}
// Reallocates and appends string s to summary_str
void ihct_add_to_summary(char *s) {
    char *p = realloc(summary_str, strlen(summary_str) + strlen(s) + 1);
    if(!p) {
        printf("Couldn't allocate a str big enough to hold summary. Aborting.\n");
        return;
    }
    summary_str = p;
    strcat(summary_str, s);
}
void ihct_add_error_to_summary(ihct_test_result *res, ihct_unit *unit) {
    char *msg;
    char *msg_format;
    size_t msg_size;
    switch (res->status) {
    case PASS: break;
    case FAIL:
        msg_format = IHCT_BOLD "%s:%d: "
            IHCT_RESET "assertion in '"
            IHCT_BOLD "%s"
            IHCT_RESET "' "
            IHCT_FG_RED "failed"
            IHCT_RESET ":\n\t'"
            IHCT_FG_YELLOW "%s"
            IHCT_RESET "'\n";
        msg_size = snprintf(NULL, 0, msg_format, res->file, res->line,
            unit->name, res->code) + 1;
        msg = calloc(msg_size, sizeof(*msg));
        sprintf(msg, msg_format, res->file, res->line, unit->name, res->code);
    break;
    case FAIL_FORCE:
        msg_format = IHCT_BOLD "%s:%d: "
            IHCT_RESET "'"
            IHCT_BOLD "%s"
            IHCT_RESET "' "
            IHCT_FG_RED "forcefully failed"
            IHCT_RESET ".\n";
        msg_size = snprintf(NULL, 0, msg_format, res->file, res->line,
            unit->name) + 1;
        msg = calloc(msg_size, sizeof(*msg));
        sprintf(msg, msg_format, res->file, res->line, unit->name);
    break;
    case ERR:
        msg_format = "unit '"
            IHCT_BOLD "%s"
            IHCT_RESET "' had to restore because of fatal signal ("
            IHCT_FG_RED "%s"
            IHCT_RESET ")\n";
        msg_size = snprintf(NULL, 0, msg_format, unit->name, res->code) + 1;
        msg = calloc(msg_size, sizeof(*msg));
        sprintf(msg, msg_format, unit->name, res->code);
    break;
    case TIMEOUT:
        msg_format = "unit '"
            IHCT_BOLD "%s"
            IHCT_RESET "' "
            IHCT_FG_YELLOW "timed out "
            IHCT_RESET "(took "
            IHCT_FG_YELLOW "5 "
            IHCT_RESET "seconds).\n";
        msg_size = snprintf(NULL, 0, msg_format, unit->name) + 1;
        msg = calloc(msg_size, sizeof(*msg));
        sprintf(msg, msg_format, unit->name);
    }
    ihct_add_to_summary(msg);

    free(msg);
}

bool ihct_assert_impl(bool eval, ihct_test_result *result, char *code, char *file,
                      unsigned long line) {
    result->status = eval ? PASS : FAIL;

    if(!eval) {
        result->file = file;
        result->line = line;
        result->code = code;
        return false;
    }
    return true;
}

void ihct_pass_impl(ihct_test_result *result, char *file, unsigned long line) {
    result->status = PASS;
    result->file = file;
    result->line = line;
}

void ihct_fail_impl(ihct_test_result *result, char *file, unsigned long line) {
    result->status = FAIL_FORCE;
    result->file = file;
    result->line = line;
}

static ihct_unit *ihct_init_unit(char *name, ihct_test_proc procedure) {
    ihct_unit *unit = (ihct_unit *)malloc(sizeof(ihct_unit));
    char *strmem = malloc(strlen(name) + 1);
    strcpy(strmem, name);
    unit->name = strmem;
    unit->procedure = procedure;

    return unit;
}

void ihct_construct_test_impl(char *name, ihct_test_proc procedure) {
    ihct_unit *unit = ihct_init_unit(name, procedure);
    ihct_vector_add(testunits, unit);
}

static void ihct_unit_free(ihct_unit *unit) {
    free(unit->name);
    free(unit);
}

void ihct_init(void) {
    // atm, only initializes the unit list. Is this neccessary?
    testunits = ihct_vector_init();
}

struct routine_run_unit_data {
    ihct_test_proc proc;
    ihct_test_result *result;
};

// Routine run in a separate thread to execute the unit.
void *routine_run_unit(void *arg) {
    struct routine_run_unit_data *data = (struct routine_run_unit_data *)arg;

    int old;
    // Allow the thread to be canceled.
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old);


    // Create a jump point, to be able to restore when encountering fatal signal.
    // When returning here because of a fatal signal, we abort unit with status
    // ERR. Has to be done inside of thread, refer to man setjmp:
    // "If, in a multithreaded program, a longjmp() call employs an env
    // buffer that was initialized by a call to setjmp() in a different
    // thread, the behavior is undefined.".
    ihct_setup_recover_action();
    int restore_status = setjmp(restore_environment);
    if(restore_status != 0) {
        char *p = malloc(strlen(strsignal(restore_status)) + 1);
        strcpy(p, strsignal(restore_status));
        data->result->code = p;
        data->result->status = ERR;

        // We still want to emit finished signal
        pthread_mutex_lock(&lock);
        pthread_cond_signal(&routine_done);
        pthread_mutex_unlock(&lock);
        return NULL;
    }


    // Run test, and save it's result.
    (*data->proc)(data->result);

    // Emit signal that thread is finished.
    pthread_mutex_lock(&lock);
    pthread_cond_signal(&routine_done);
    pthread_mutex_unlock(&lock);
    return NULL;
}

static ihct_test_result *ihct_run_specific(ihct_unit *unit) {
    // Allocate memory for the tests result, and set it to passed by default.
    ihct_test_result *result = malloc(sizeof(ihct_test_result));
    result->status = PASS;

    // lock current thread.
    pthread_mutex_lock(&lock);

    // Create a separate thread to run the test in. We set a limited time
    // the process may be run, and abort if it times out.
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += test_timeout;

    // Create a temporary data struct to carry data into thread.
    struct routine_run_unit_data data = {unit->procedure, result};

    // Create new thread to run the unit routine.
    pthread_t tid;
    pthread_cond_init(&routine_done, NULL);
    pthread_create(&tid, NULL, routine_run_unit, &data);

    int err =  pthread_cond_timedwait(&routine_done, &lock, &timeout);

    pthread_mutex_unlock(&lock);

    // If timed out, force quit thread and return TIMEOUT.
    // Note that this is not safe memory. There is a high chance that
    // the thread may not be freed. Looking into solving this.
    if(err == ETIMEDOUT) {
        pthread_cancel(tid);

        result->status = TIMEOUT;
        return result;
    }

    //pthread_join(procedure, NULL);
    pthread_join(tid, NULL);

    // Run test, and save it's result into i.
    //(*unit->procedure)(result);

    return result;
}

int ihct_run(int argc, char **argv) {
    unsigned unit_count = testunits->size;
    // Allocate results
    ihct_results = calloc(unit_count, sizeof(ihct_test_result *));

    unsigned failed_count = 0;

    // initialize the summary string
    summary_str = malloc(sizeof(char));
    *summary_str = '\0';

    // handle args
    int c;
    while((c = getopt(argc, argv, "t:")) != -1) {
        switch(c) {
        case 't':
            test_timeout = atoi(optarg);
            break;
        case '?':
            printf("unknown option '%c'.\n", optopt);
        }
    }

    // start clock
    struct timespec tbegin, tend;
    clock_gettime(CLOCK_MONOTONIC, &tbegin);

    // Iterate over every test
    for(unsigned i = 0; i < unit_count; i++) {
        ihct_unit *unit = ihct_vector_get(testunits, i);

        ihct_results[i] = ihct_run_specific(unit);

        // ensure 80 width
        if(i % 80 == 0 && i != 0) putc('\n', stdout);

        ihct_print_result(ihct_results[i]);
        fflush(stdout);

        if(ihct_results[i]->status) {
            failed_count++;
            ihct_add_error_to_summary(ihct_results[i], unit);
        }
        // Frees both the unit and the result.
        ihct_unit_free(unit);

        // Also frees dynamic allocated string if status is err (the signal name).
        if(ihct_results[i]->status == ERR) free(ihct_results[i]->code);

        free(ihct_results[i]);
    }
    free(ihct_results);
    ihct_vector_free(testunits);

    clock_gettime(CLOCK_MONOTONIC, &tend);
    double elapsed = (tend.tv_sec - tbegin.tv_sec);
    elapsed += (tend.tv_nsec - tbegin.tv_nsec) / 1000000000.0;

    // print all messages
    if(strlen(summary_str) > 4) printf("\n\n%s\n", summary_str);
    else puts("\n\n");

    // Free summary str
    free(summary_str);

    printf("tests took %.2f seconds\n", elapsed);
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

// Create two internal test procedures, for testing the test creation.
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