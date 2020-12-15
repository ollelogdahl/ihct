#include "ihct.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
    // Run test, and save it's result into i.
    (*unit->procedure)(result);

    return result;
}

int ihct_run(int argc, char **argv) {
    unsigned unit_count = testunits->size;
    // Allocate results
    ihct_results = calloc(unit_count, sizeof(ihct_test_result *));

    unsigned failed_count = 0;

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

    clock_t time_posttests = clock();
    double elapsed = (double)(time_posttests - time_pretests) / CLOCKS_PER_SEC;

    for(unsigned i = 0; i < unit_count; ++i) {
        ihct_unit *unit = ihct_vector_get(testunits, i);

        if(!ihct_results[i]->passed) {
            char *assertion_format = IHCT_BOLD "%s:%d: "
                IHCT_RESET "assertion in '"
                IHCT_BOLD "%s"
                IHCT_RESET "' failed:\n\t'"
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

IHCT_TEST(self_minimal) {
    IHCT_ASSERT(1 == 1);
}

IHCT_TEST(self_unit_create) {
    ihct_unit *u = ihct_init_unit("test", &test_self_minimal);
    IHCT_ASSERT_STR(u->name, "test");
    IHCT_ASSERT(&test_self_minimal == u->procedure);
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