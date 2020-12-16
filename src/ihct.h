#ifndef IHCT_H
#define IHCT_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Structure for a testunits return value. Contains state, the code (assert) which
// failed the test, and a reference to where the code is.
typedef struct {
    enum {PASS, FAIL, ERR, TIMEOUT} status;
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
static ihct_unit *ihct_init_unit(char *name, ihct_test_proc procedure);

// Frees the created unit (does not remove it from the unit list).
static void ihct_unit_free(ihct_unit *unit);

// Called within a test. 
bool ihct_assert_impl(bool eval, ihct_test_result *result, char *code, char *file, 
                      unsigned long line);

// Called on test unit construction.
void ihct_construct_test_impl(char *s, ihct_test_proc proc);

// Runs all tests.
int ihct_run(int argc, char **argv);

// Initializes the unitlist (Has to be done before all testing units are created).
// Using priority to ensure that the unit list is constructed before it gets populated.
static void ihct_init(void) __attribute__((constructor(101)));

// Run a specific testing unit.
static ihct_test_result *ihct_run_specific(ihct_unit *unit);

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


// Assertions
/// @defgroup assertions Assertions
/// @brief Wraps all assertions.
///
/// Decides the success of a test. Right now, tests can only result in
/// PASS or FAIL. Assertions are only given for expressions.

/// @brief Asserts a statement inside a test unit. If the expression is false,
/// the unit will fail the test.
/// @ingroup assertions
/// @param expr the expression to evaluate.
#define IHCT_ASSERT(expr)                                                               \
    if(!ihct_assert_impl(expr, result, #expr, __FILE__, __LINE__)) return

/// @brief Asserts a statement inside a test unit. If the expression is true,
/// the unit will fail the test.
/// @ingroup assertions
/// @param expr the expression to evaluate.
#define IHCT_NASSERT(expr)                                                              \
    if(!ihct_assert_impl(!expr, result, "!" #expr, __FILE__, __LINE__)) return

/// @brief Asserts two strings inside a test unit to be equal. If there is any difference
/// in the strings, the unit will fail the test.
/// @ingroup assertions
/// @param s1 first string to compare
/// @param s2 second string to compare
#define IHCT_ASSERT_STR(s1, s2)                                                         \
    if(!ihct_assert_impl(!strcmp(s1, s2), result, #s1 " == " #s2, __FILE__,             \
       __LINE__)) return
/// @brief Asserts two strings inside a test unit not to be equal. If there is any 
/// difference in the strings, the unit will fail the test.
/// @ingroup assertions
/// @param s1 first string to compare
/// @param s2 second string to compare
#define IHCT_NASSERT_STR(s1, s2)                                                        \
    if(!ihct_assert_impl(strcmp(s1, s2), result, #s1 " != " #s2, __FILE__,              \
       __LINE__)) return

/// @brief Set the test as passed and return.
/// @ingroup assertions
///
/// Used for more complex tests where the PASS/FAIL status is more complex
/// than an assert.
#define IHCT_PASS()                                                                     \
    result->status = PASS; return

/// @brief Set the test as failed and return.
/// @ingroup assertions
///
/// Used for more complex tests where the PASS/FAIL status is more complex
/// than an assert.
#define IHCT_FAIL()                                                                     \
    result->status = FAIL; return

// Function macros
/// @defgroup funcs Testing functions
/// @brief More general macros for function.

/// @brief Runs all tests. Is to be called once in the main entrypoint.
/// @ingroup funcs
/// @code
/// int main(int argc, char **argv) {
///     return IHCT_RUN(argc, argv);
/// }
/// @endcode
/// @param argc argument count, directly passed from main.
/// @param argv argument array, directly passed from main.
#define IHCT_RUN(argc, argv)                                                            \
    ihct_run(argc, argv)

// Create a new test unit, and adds it using 'ihct_add_test'.
/// @brief Create a new test unit, which can take any number of asserts.
/// @ingroup funcs
/// @code
/// IHCT_TEST(basic_test)
/// @param name the name of the test.
#define IHCT_TEST(name)                                                                 \
    static void test_##name(ihct_test_result *result);                                  \
    static void __attribute__((constructor(102))) __construct_test_##name(void) {       \
        ihct_construct_test_impl(#name, &test_##name);                                   \
    }                                                                                   \
    static void test_##name(ihct_test_result *result)

/// @brief Defines a fixture with data to be preloaded before a test.
/// A ficture is included by a IHCT_REQUIRE inside a test.
#define IHCT_FIXTURE(name) blah

/// @brief Make the test require the given fixtures.
/// @param ... one or more fixture names.
#define IHCT_REQUIRE(...) lel

#endif