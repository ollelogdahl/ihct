#include "ihct.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

IHCT_TEST(arithmetic_addition_basic) {
    IHCT_ASSERT(1 + 2 == 3);
    IHCT_ASSERT(4 + 2 == 6);
    IHCT_ASSERT(8 + 8 == 16);
}
IHCT_TEST(arithmetic_addition_big) {
    IHCT_ASSERT(1000 + 2000 == 3000);
    IHCT_ASSERT(4000 + 2000 == 6000);
    IHCT_ASSERT(8000 + 8000 == 16000);
}
IHCT_TEST(arithmetic_multiplication) {
    IHCT_ASSERT(1 * 3 == 3);
    IHCT_ASSERT(0 * 3 == 0);
    IHCT_ASSERT(3 * 3 == 9);
}

IHCT_TEST(strings_basic) {
    IHCT_NASSERT(strcmp("abba", "abba"));
    IHCT_ASSERT(strcmp("abba", "Abba"));
}

int main(int argc, char **argv) {
    return IHCT_RUN(argc, argv);
}