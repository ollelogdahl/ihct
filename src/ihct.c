#include "ihct.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ihct_unitlist *unit_list;

// An array of all first failed (or last if all successful) assert results in every test.
struct test_result **ihct_results;

bool ihct_assert_impl(bool eval, struct test_result *result, char *code, char *file, unsigned long line) {
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

static void ihct_init_unitlist() {
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

void ihct_init(void) __attribute__((constructor));
void ihct_init(void) {
   // atm, only initializes the unit list. Is this neccessary?
   ihct_init_unitlist();
}

int ihct_run(int argc, char **argv) {
    // Allocate results
    ihct_results = calloc(unit_list->size, sizeof(struct test_result*));

    unsigned failed_count = 0;

    ihct_unitlist_node *cur = unit_list->head;
    for(unsigned i = 0; i < unit_list->size; i++) {
        cur = cur->next;

        // Allocate memory for the result of the test.
        struct test_result *mem = malloc(sizeof(struct test_result));
        ihct_results[i] = mem;

        // Run test, and save it's status into i.
        (*cur->unit->procedure)(ihct_results[i]);

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

    printf("\n");
    if(failed_count) {
        char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
            IHCT_RESET "and "
            IHCT_FOREGROUND_RED "%d failed "
            IHCT_RESET "of "
            IHCT_FOREGROUND_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, unit_list->size-failed_count, failed_count, unit_list->size);
    } 
    else {
        char *status_format = IHCT_FOREGROUND_GREEN "%d successful "
            IHCT_RESET "of "
            IHCT_FOREGROUND_YELLOW "%d run"
            IHCT_RESET "\n";
        printf(status_format, unit_list->size, unit_list->size);
    }

    free(ihct_results);

    if(failed_count) {
        printf(IHCT_FOREGROUND_RED "FAILURE\n" IHCT_RESET);
        return 1;
    }
    printf(IHCT_FOREGROUND_GREEN "SUCCESS\n" IHCT_RESET);
    return 0;
}