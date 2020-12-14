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

    for(int i = 0; i < 8000000; ++i) {
        int a = i * 14;
        int b = i * 51;
        int c = i * 221;
        ihct_vector_add(v, &a);
        ihct_vector_add(v, &b);
        ihct_vector_add(v, &c);

        IHCT_ASSERT(&a == ihct_vector_get(v, 0));
        IHCT_ASSERT(&b == ihct_vector_get(v, 1));
        IHCT_ASSERT(&c == ihct_vector_get(v, 2));
    }

    ihct_free_vector(v);
}

int main(int argc, char **argv) {
    return IHCT_RUN(argc, argv);
}