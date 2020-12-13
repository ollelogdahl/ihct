#include "ihct.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ihct_unitlist *unit_list;

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

void ihct_init_unit(char *name, ihct_test_proc procedure) {
    printf("Constructing new unit: %s\n", name);

    ihct_unit *unit = (ihct_unit *)malloc(sizeof(ihct_unit));
    char *strmem = malloc(strlen(name));
    strcpy(strmem, name);
    unit->name = strmem;
    unit->procedure = procedure;

    // Add unit to list
    // unit_list->next = 
}

int ihct_run(int argc, char **argv) {
    printf("Running main.\n");
    return 0;
}