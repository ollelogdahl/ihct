#ifndef IHCT_H
#define IHCT_H

#include <stdbool.h>

// Structure for a testunits return value. Contains state, the code (assert) which
// failed the test, and a reference to where the code is.
typedef struct {
    bool passed;
    char *code;
    char *file;
    unsigned long line;
} ihct_test_result;

// Short for a function returning a test_result pointer, with no arguments.
typedef void (*ihct_test_proc)(ihct_test_result *);

// Object representing a testing unit, containing the units name and its procedure
// (implemented test function).
typedef struct {
    char *name;
    ihct_test_proc procedure;
} ihct_unit;

// Allocates a new unit node.
ihct_unit *ihct_init_unit(char *name, ihct_test_proc procedure);
// Frees the created unit (does not remove it from the unit list).
static void ihct_unit_free(ihct_unit *unit);

// Basic linked-list implementation for listing all units.
// Structure representing a single node in the testing unit list.
typedef struct ihct_unitlist_node {
    struct ihct_unitlist_node *next;
    ihct_unit *unit;
} ihct_unitlist_node;

// Structure representing a list of testing units. NOTE: program only contains one
// instance of this list, where the implementation is specified to that instance.
typedef struct {
    ihct_unitlist_node *head;
    unsigned size;
} ihct_unitlist;

// Initializes the list.
static void ihct_init_unitlist(void);
// Adds a node to the list. Does this in a first-in style, because the list is only 
// forward linked. TODO: could we make it last-in, is there any point to it?
void ihct_unitlist_add(ihct_unit *unit);
// Frees the list and all contained nodes. Also calls ihct_unit_free.
static void ihct_unitlist_free(void);

// Called within a test. 
bool ihct_assert_impl(bool eval, ihct_test_result *result, char *code, char *file, 
                      unsigned long line);
// Runs all tests.
int ihct_run(int argc, char **argv);
// Initializes the unitlist (Has to be done before all testing units are created).
// Using priority to ensure that the unit list is constructed before it gets populated.
void ihct_init(void) __attribute__((constructor(101)));

// Run a specific testing unit.
ihct_test_result *ihct_run_specific(ihct_unit *unit);


// These are ISO/IEC 6429 escape sequences for
// communicating text attributes to terminal emulators.
// Note that some compilers do not understand '\x1b', and therefore \033[0m is 
// used instead.
#define IHCT_RESET "\033[0m"
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
#define IHCT_ASSERT(stmnt) \
    if(!ihct_assert_impl(stmnt, result, #stmnt, __FILE__, __LINE__)) return
#define IHCT_NASSERT(stmnt) \
    if(!ihct_assert_impl(!stmnt, result, #stmnt, __FILE__, __LINE__)) return

// Function macros
#define IHCT_RUN(argc, argv) ihct_run(argc, argv)

// Create a new test unit, and adds it using 'ihct_add_test'.
#define IHCT_TEST(name)\
    static void test_##name(ihct_test_result *result); \
    static void __attribute__((constructor(102))) __construct_test_##name(void) { \
        ihct_unit *unit = ihct_init_unit(#name, test_##name); \
        ihct_unitlist_add(unit); \
    } \
    static void test_##name(ihct_test_result *result)

#endif