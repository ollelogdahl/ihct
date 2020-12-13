# ihct
### I hate c testing: a minimal testing framework
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
```