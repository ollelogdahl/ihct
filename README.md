# ihct
### 'I Hate C Testing': a minimal testing framework
Olle Lögdahl, 13 December 2020

---

### Usage

```c
#include <ihct.h>

IHCT_TEST(arithmetic_basic) {
    int a = 13;
    IHCT_ASSERT(a + 2 == 15);
    IHCT_ASSERT(a - 4 == 9);
    IHCT_ASSERT(a * 2 == 26);
}
IHCT_TEST(string_basic) {
    char *s1 = "abba";
    IHCT_NASSERT(strcpr(s1, "abba"));
    IHCT_ASSERT(strcpr(s1, "Abba"));
}
```