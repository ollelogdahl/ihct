# ihct
### 'I Hate C Testing': a minimal testing framework for C
Olle Lögdahl, 13 December 2020

![make](https://img.shields.io/github/workflow/status/ollelogdahl/ihct/build)
![docs](https://img.shields.io/github/workflow/status/ollelogdahl/ihct/Doxygen?label=docs)
![licence](https://img.shields.io/github/license/ollelogdahl/ihct)
![issues](https://img.shields.io/github/issues-raw/ollelogdahl/ihct)

---

**ihct** is a minimal C unit-testing framework. Intended for light unit testing, and keeping the user interface
logical and light. Looking for more features, so please give any suggestions.

## Installation & Usage

#### See [documentation](https://ollelogdahl.github.io/ihct/)

To use this framework, include `ihct.h` and `ihct.c` in your project. See `ex.c` for an extended example.
The test creates it's own executable, and therefore needs an entrypoint. The following code should get you started.

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

int main(int argc, char **argv) {
    return IHCT_RUN(argc, argv);
}
```

The example `ex.c` can be compiled with by running:
```bash
mkdir build
cd build
cmake .. && make -j4
```

Self tests can be run along with own tests by adding compiler flag `-DIHCT_SELF_TEST`. (This may be very redundant; just see it as more examples :-) )

---

## Features
- Basic test units
- Basic asserts
- Automatic test loader
- Catching fatal signals (SEGFAULTS etc.) in tests (no line number, but sets them as failed).
- Catching hung tests (again, no line number).

## Links

- [Repository](https://github.com/ollelogdahl/ihct/)
- For reporting errors, visit [Issue Tracker](https://github.com/ollelogdahl/ihct/issues)!
- Related Projects:
  - [cheat](https://github.com/Tuplanolla/cheat) by Guillermo "Tordek" Freschi and Sampsa "Tuplanolla" Kiiskinen

## Licensing

This project, and all code it contains, is licensed under the *MIT License* and can be read [here](LICENSE).
