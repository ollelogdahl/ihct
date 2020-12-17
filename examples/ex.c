#include "ihct.h"

IHCT_TEST(arithmetic_addition_basic) {
    IHCT_ASSERT(1 + 2 == 3);
    IHCT_ASSERT(4 + 2 == 6);
    IHCT_ASSERT(8 + 8 == 16);
}
IHCT_TEST(arithmetic_addition_big) {
    IHCT_NASSERT(1000 + 1 == 1001);
}
IHCT_TEST(arithmetic_multiplication) {
    IHCT_ASSERT(1 * 3 == 4);
}

IHCT_TEST(timeout_slow) {
    for(;;) {
        int i = 1;
    }
}

IHCT_TEST(sigsegv_test) {
    int *p = NULL;
    *p = 3;
}

IHCT_TEST(strings_basic) {
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

IHCT_TEST(timeout_test) {
    for(;;) {
        int i = 1;
    }
}

IHCT_TEST(if_no_block) {
    if(0) IHCT_ASSERT(1 == 1);
    
    IHCT_FAIL();
}

int main(int argc, char **argv) {
    return IHCT_RUN(argc, argv);
}
