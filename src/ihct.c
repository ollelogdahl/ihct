#include "ihct.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// A list of all units.
ihct_unitlist *unit_list;

// An array of all first failed (or last if all successful) assert results in every test.
ihct_test_result **ihct_results;

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

ihct_unit *ihct_init_unit(char *name, ihct_test_proc procedure) {
    //printf("Constructing new unit: %s\n", name);

    ihct_unit *unit = (ihct_unit *)malloc(sizeof(ihct_unit));
    char *strmem = malloc(strlen(name));
    strcpy(strmem, name);
    unit->name = strmem;
    unit->procedure = procedure;
    //ihct_unitlist_add(node);

    return unit;
}

void ihct_unit_free(ihct_unit *unit) {
    free(unit->name);
    free(unit);
}

static void ihct_init_unitlist(void) {
    // initialize the unit_list head, which points at itself
    // and doesn't have a unit.
    unit_list = malloc(sizeof(ihct_unitlist));

    ihct_unitlist_node *head = malloc(sizeof(ihct_unitlist_node));
    head->next = head;
    head->unit = NULL;

    unit_list->head = head;
    unit_list->size = 0;
}

void ihct_unitlist_add(ihct_unit *unit) {
    printf("Called add unit\n");
    // Before:          After
    // ┌────┐   ┌─┐     ┌────┐   ┌────┐   ┌─┐
    // │head│-->│a|     │head│-->│node|-->│a|
    // └────┘   └─┘     └────┘   └────┘   └─┘

    // Allocate a new node
    ihct_unitlist_node *node = malloc(sizeof(ihct_unitlist_node));

    // Assign pointers
    node->next = unit_list->head->next;
    unit_list->head->next = node;
    unit_list->head->next->unit = unit;
    unit_list->size++;
}

void ihct_init(void) {
    printf("Called init\n");
    // atm, only initializes the unit list. Is this neccessary?
    ihct_init_unitlist();
}

static void ihct_unitlist_free(void) {
    ihct_unitlist_node *cur = unit_list->head;
    for(unsigned i = 0; i < unit_list->size; i++) {
        ihct_unitlist_node *prev = cur;
        cur = cur->next;

        ihct_unit_free(cur->unit);
        free(prev);
    }
    free(unit_list);
}


ihct_test_result *ihct_run_specific(ihct_unit *unit) {
    // Allocate memory for the tests result.

    ihct_test_result *result = malloc(sizeof(ihct_test_result));
    // Run test, and save it's result into i.
    (*unit->procedure)(result);

    return result;
}

int ihct_run(int argc, char **argv) {
    // Allocate results
    ihct_results = calloc(unit_list->size, sizeof(ihct_test_result*));

    unsigned failed_count = 0;

    ihct_unitlist_node *cur = unit_list->head;
    for(unsigned i = 0; i < unit_list->size; i++) {
        cur = cur->next;

        ihct_results[i] = ihct_run_specific(cur->unit);

        if(ihct_results[i]->passed) {
            printf(IHCT_BACKGROUND_GREEN IHCT_BOLD "." IHCT_RESET);
        } else {
            printf(IHCT_BACKGROUND_RED IHCT_BOLD "!" IHCT_RESET);
        }
    }
    printf("\n\n");

    for(unsigned i = 0; i < unit_list->size; ++i) {
        if(!ihct_results[i]->passed) {
            char *assertion_format = IHCT_BOLD "%s:%d: "
                IHCT_RESET "assertion in '"
                IHCT_BOLD "%s"
                IHCT_RESET "' failed:\n\t'"
                IHCT_FOREGROUND_YELLOW "%s"
                IHCT_RESET "'\n";
            printf(assertion_format, ihct_results[i]->file, ihct_results[i]->line, 
                cur->unit->name, ihct_results[i]->code);
            failed_count++;
        }
    }

    free(ihct_results);
    ihct_unitlist_free();

    printf("\n");
    if(failed_count) {
        char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
            IHCT_RESET "and "
            IHCT_FOREGROUND_RED "%d failed "
            IHCT_RESET "of "
            IHCT_FOREGROUND_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, unit_list->size-failed_count, failed_count, 
               unit_list->size);
    } 
    else {
        char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
            IHCT_RESET "of "
            IHCT_FOREGROUND_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, unit_list->size, unit_list->size);
    }

    if(failed_count) {
        printf(IHCT_FOREGROUND_RED "FAILURE\n" IHCT_RESET);
        return 1;
    }
    printf(IHCT_FOREGROUND_GREEN "SUCCESS\n" IHCT_RESET);
    return 0;
}