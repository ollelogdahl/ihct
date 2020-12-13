#ifndef IHCT_H
#define IHCT_H

#include <stdbool.h>

// Test result
typedef struct test_result {
    bool passed;
    char *code;
    char *file;
    unsigned long line;
} test_result;

typedef void (*ihct_test_proc)(test_result *);
typedef struct {
    const char *name;
    ihct_test_proc procedure;
} ihct_unit;

// Basic linked-list implementation for listing all units.
typedef struct ihct_unitlist_node {
    struct ihct_unitlist_node *next;
    ihct_unit *unit;
} ihct_unitlist_node;
typedef struct ihct_unitlist {
    ihct_unitlist_node *head;
    unsigned size;
} ihct_unitlist;


bool ihct_assert_impl(bool eval, struct test_result *result, char *code, char *file, unsigned long line);
int ihct_run(int argc, char **argv);
void ihct_init(void);

// Create and appends new unit.
void ihct_init_unit(char *name, ihct_test_proc procedure);


/*
These are ISO/IEC 6429 escape sequences for
communicating text attributes to terminal emulators.
*/
#define IHCT_RESET "\033[0m" // Some compilers do not understand '\x1b'.
#define IHCT_BOLD "\033[1m"
#define IHCT_FOREGROUND_GRAY "\033[30;1m"
#define IHCT_FOREGROUND_RED "\033[31;1m"
#define IHCT_FOREGROUND_GREEN "\033[32;1m"
#define IHCT_FOREGROUND_YELLOW "\033[33;1m"
#define IHCT_FOREGROUND_BLUE "\033[34;1m"
#define IHCT_FOREGROUND_MAGENTA "\033[35;1m"
#define IHCT_FOREGROUND_CYAN "\033[36;1m"
#define IHCT_FOREGROUND_WHITE "\033[37;1m"
#define IHCT_BACKGROUND_BLACK "\033[40;1m"
#define IHCT_BACKGROUND_RED "\033[41;1m"
#define IHCT_BACKGROUND_GREEN "\033[42;1m"
#define IHCT_BACKGROUND_YELLOW "\033[43;1m"
#define IHCT_BACKGROUND_BLUE "\033[44;1m"
#define IHCT_BACKGROUND_MAGENTA "\033[45;1m"
#define IHCT_BACKGROUND_CYAN "\033[46;1m"
#define IHCT_BACKGROUND_GRAY "\033[47;1m"


// Assertions
#define IHCT_ASSERT(stmnt)\
    if(!ihct_assert_impl(stmnt, result, #stmnt, __FILE__, __LINE__)) return
#define IHCT_NASSERT(stmnt)\
    if(!ihct_assert_impl(!stmnt, result, #stmnt, __FILE__, __LINE__)) return

// Function macros
#define IHCT_RUN(argc, argv) ihct_run(argc, argv)
#define IHCT_INIT() ihct_init()

// Create a new test unit, and adds it using 'ihct_add_test'.
#define IHCT_TEST(name)\
    static void test_##name(struct test_result *result); \
    static void __attribute__((constructor)) __construct_test_##name(void) { \
        ihct_init_unit(#name, test_##name); \
    } \
    static void test_##name(struct test_result *result)

#endif