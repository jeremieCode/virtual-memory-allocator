# Virtual Memory Allocator

A user-space dynamic memory allocator implemented in C, providing `emalloc` and `efree` as drop-in replacements for `malloc` / `free`.

It handles three allocation strategies depending on request size, uses a double-ended block marking scheme for overflow detection, and is thread-safe via a POSIX mutex.

---

## What this implements

Three size classes, each with a distinct mechanism:

- **Small** (`<= 64` bytes): fixed-size chunk pool, singly linked free list,
  refilled on demand with exponential doubling
- **Medium** (`64` bytes to `128 KiB`): buddy (binary subdivision) algorithm
  with coalescing on free
- **Large** (`>= 128 KiB`): one `mmap` per allocation, `munmap` on free

Every block is surrounded by a header and footer storing its total size and a magic word derived from its address. This enables buffer overflow detection at free time: the same mechanism that causes the system `free()` to crash when memory has been corrupted.

Thread safety is provided by a single POSIX mutex (`pthread_mutex_t`) protecting the shared arena. Cross-thread frees are supported.

## Project structure

```
.
├── report.md                       # design and implementation notes
├── problem-statement.pdf           # subject statement (French)
├── screenshots/
├── src/
│   ├── mem.h                       public API: emalloc, efree
│   ├── mem_internals.h             internal types, constants, arena structure
│   ├── mem.c                       dispatcher, arena declaration, mutex
│   ├── mem_internals.c             block marking, mmap helpers
│   ├── mem_small.c                 small pool allocator
│   ├── mem_medium.c                buddy allocator
│   ├── mem_large.c                 large mmap allocator
└── tests/
```

## Building and testing

```sh
cd build
cmake ..
make
make test       # run the full test suite
make check      # verbose output
```

## References

Subject statement and all course material for `Operating Systems and Concurrent Programming`: https://systemes.gricad-pages.univ-grenoble-alpes.fr/sepc/ (in french)
