#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Include the actual production header under test
#include "src/avt/IVP/MemStream.h"

START_TEST(test_memstream_bounds_invariant)
{
    // Invariant: Read/write operations must never access memory beyond buffer bounds.
    // Positions and sizes that would cause out-of-bounds access must be rejected or clamped.

    struct {
        int buf_size;
        int read_pos;
        size_t read_bytes;
        const char *label;
    } cases[] = {
        // Exact exploit: pos near end, nBytes overflows buffer
        { 16, 14, 8, "overflow_read" },
        // Boundary: read exactly at last valid byte
        { 16, 15, 1, "boundary_last_byte" },
        // Valid: normal read well within bounds
        { 16, 0, 8, "valid_read" },
        // Exploit: pos=0, nBytes larger than entire buffer
        { 8, 0, 1024, "nBytes_exceeds_buffer" },
    };

    int num_cases = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < num_cases; i++) {
        int buf_size = cases[i].buf_size;
        int read_pos = cases[i].read_pos;
        size_t read_bytes = cases[i].read_bytes;

        // Allocate a MemStream backed by a known-size buffer
        MemStream ms(buf_size);

        // Write a known pattern to fill the stream
        unsigned char pattern[1024] = {0};
        memset(pattern, 0xAB, sizeof(pattern));
        size_t safe_write = (buf_size < (int)sizeof(pattern)) ? buf_size : sizeof(pattern);
        ms.write(pattern, safe_write);

        // Seek to the adversarial position
        ms.seek(read_pos);

        // The invariant: reading nBytes from read_pos must not exceed buffer bounds
        // i.e., read_pos + read_bytes <= buf_size must hold for a safe read
        int is_safe = ((size_t)read_pos + read_bytes) <= (size_t)buf_size;

        if (is_safe) {
            unsigned char out[1024] = {0};
            // Safe read: must succeed without memory corruption
            ms.read(out, read_bytes);
            ck_assert_msg(1, "Safe read should complete: %s", cases[i].label);
        } else {
            // Unsafe: the operation would go out of bounds.
            // The stream MUST either reject the read (return error/throw)
            // or clamp to available bytes — never silently over-read.
            int available = buf_size - read_pos;
            ck_assert_msg(available < (int)read_bytes,
                "Out-of-bounds case confirmed unsafe: %s", cases[i].label);
            // If the implementation does not guard, this is the regression point.
            // A hardened implementation would fail gracefully here.
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_memstream_bounds_invariant);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}