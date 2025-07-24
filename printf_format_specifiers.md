================= C printf() Format Specifiers =================

Type/Format         | Specifier | Example Arg       | Example Output
--------------------|-----------|-------------------|------------------
Signed int          | %d        | -42               | -42
Unsigned int        | %u        | 42                | 42
Signed long         | %ld       | -42000L           | -42000
Unsigned long       | %lu       | 42000UL           | 42000
Signed long long    | %lld      | -123456789LL      | -123456789
Unsigned long long  | %llu      | 123456789ULL      | 123456789
Hex (lower/upper)   | %x / %X   | 0x1A              | 1a / 1A
Hex with prefix     | %#x       | 0x1A              | 0x1a
Octal               | %o        | 0755              | 755
Character           | %c        | 'A'               | A
C-string            | %s        | "hello"           | hello
Pointer             | %p        | &var              | 0x400d1234
Float/double        | %f        | 3.14              | 3.140000
Float (2 decimals)  | %.2f      | 3.14              | 3.14
size_t              | %zu       | size_t size       | 42
ssize_t             | %zd       | (signed size_t)   | -5
Literal percent     | %%        | -                 | %

Note: %f requires CONFIG_NEWLIB_LIBC_FLOAT_PRINTF=y in ESP-IDF.

===============================================================
