# ihct
### 'I Hate C Testing': a minimal testing framework for C
Olle Lögdahl, 13 December 2020

![make](https://img.shields.io/github/workflow/status/ollelogdahl/ihct/build)
![docs](https://img.shields.io/github/workflow/status/ollelogdahl/ihct/Doxygen?label=docs)
![licence](https://img.shields.io/github/license/ollelogdahl/ihct)
![issues](https://img.shields.io/github/issues-raw/ollelogdahl/ihct)
![sloc](https://raw.githubusercontent.com/ollelogdahl/ihct/image-data/badge.svg)

---

**ihct** is a minimal C unit-testing framework. Intended for light unit testing, and focusing on development speed. 
Looking for more features, so please give any suggestions.

<p align="center">
    <img src="https://github.com/ollelogdahl/ihct/blob/master/media.png">
</p>

## Installation & Usage

#### See [documentation](https://ollelogdahl.github.io/ihct/)

To use this framework, simply include `ihct.h` in your project and link to the library. **Everything else happens automatically**.
The following code should get you started.

```c
#include <ihct.h>

IHCT_TEST(arithmetic_basic) {
    int a = 13;
    IHCT_ASSERT(a + 2 == 15);
    IHCT_ASSERT(a * 2 == 26);
}
IHCT_TEST(string_basic) {
    char *s1 = "abba";
    IHCT_ASSERT_STR(s1, "abba");
}

int main(int argc, char **argv) {
    return IHCT_RUN(argc, argv);
}
```

To fully install the library, run:
```bash
mkdir build
cd build
cmake .. && make -j4
sudo make install
```

See `ex.c` for an extended example. Note that tests are created as it's own executable, and therefore needs an entrypoint.
The example `ex.c` can be compiled and executed by running:
```bash
mkdir build
cd build
cmake .. && make -j4
./example
```

---

## Why?
I have for a long time been stuck at unit testing in plain C. Many modern solutions use C++ as a test environment for C, but I wanted something
more lightweight, that i can quickly get up to speed with. I decided to write my own test framework with two things in mind: **development speed** 
and **minimalism**. To improve development speed, all test functions are automatically included into the runner, and the library interface is kept
minimal. It requires no dependencies other than a POSIX compliant OS, and compiles on all GNU C99 POSIX compatible compilers. The library also 
implements some safety to tests, catching fatal signals and hung functions.

---

## Features
- Basic test units
- Basic asserts
- Automatic test loader
- Catching fatal signals (SEGFAULTS etc.) in tests (no line number, but sets them as failed).
- Catching hung tests (again, no line number).

Self tests can be run along with own tests by adding compiler flag `-DIHCT_SELF_TEST`. (This may be very redundant; just see it as more examples :-) )

all macros (`IHCT_TEST`, `IHCT_ASSERT` etc.) can be shortened to remove the `IHCT` prefix, by defining `IHCT_SHORT` **before**
including the header file.

## Compatability notes
 - Since it requires `__attribute__((constructor))` it is not compilable with MSVC.
 - Since it uses pthreads and signals, it is POSIX (again, not sure it works with Windows).

## Links

- [Repository](https://github.com/ollelogdahl/ihct/)
- For reporting errors, visit [Issue Tracker](https://github.com/ollelogdahl/ihct/issues)!
- Related Projects:
  - [cheat](https://github.com/Tuplanolla/cheat) by Guillermo "Tordek" Freschi and Sampsa "Tuplanolla" Kiiskinen

## Licensing

This project, and all code it contains, is licensed under the *MIT License* and can be read [here](LICENSE).
