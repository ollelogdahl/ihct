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

    IHCT_ASSERT_STR("abba", "abba");
    IHCT_NASSERT_STR("Alfa", "adolf");
}

IHCT_TEST(strings_invalid) {
    IHCT_ASSERT_STR("Evil", "Good"); // should fail.
}

IHCT_TEST(strings_more) {
    IHCT_ASSERT_STR("aaa", "aaa");
    IHCT_ASSERT_STR("bbb", "bbb");
    IHCT_ASSERT_STR("ccc", "ccc");
    IHCT_ASSERT_STR("ddd", "ddd");
    IHCT_ASSERT_STR("eee", "eee");
}

IHCT_TEST(self_vector_all) {
    ihct_vector *v = ihct_init_vector();

    for(int i = 0; i < 9000000; ++i) {
        int *t = malloc(sizeof(int));
        *t = i * 2;
        ihct_vector_add(v, t);
    }

    for(int i = 0; i < 9000000; ++i) {
        int *t = ihct_vector_get(v, i);
        IHCT_ASSERT(i * 2 == *t);
        free(t);
    }

    ihct_free_vector(v);
}

int main(int argc, char **argv) {
    return IHCT_RUN(argc, argv);
}