#include "../lib/common/include/testing/utils.h"
#include "../src/system.h"

define_test(system, test_print("System")) 
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    system_t sys;
    byte bs, be;
    word ws, we;
    tetra ts, te;

    bs = 0x0;
    be = 0x4;

    we = 0x3344;
    ws = 0x0000;

    ts = 0x11223344;
    te = 0x00000000;

    __sys_init(&sys, &allocator);
    sys_add_memory(&sys, &allocator, (void*) 0x00, 4);

    test_check(
        test_print("Store a byte (%d) in the system's memory.", be),
        sys_store_byte(&sys, 0x00, be),
        test_failure("Failed to store a byte...")
    );

    test_check(
        test_print("Load a byte from the system's memory."),
        sys_load_byte(&sys, 0x00, &bs),
        test_failure("Failed to load a byte...")
    );

    test_check(
        test_print("Compare the stored byte."),
        bs == be,
        test_failure("Expecting %d, got %d", be, bs)
    );

    // Reset
    sys_store_byte(&sys, 0x00, 0);

    test_check(
        test_print("Store a word (%d) in the system's memory.", we),
        sys_store_word(&sys, 0x00, we),
        test_failure("Failed to store a word...")
    );

    test_check(
        test_print("Load a word from the system's memory."),
        sys_load_word(&sys, 0x00, &ws),
        test_failure("Failed to load a word...")
    );

    test_check(
        test_print("Compare the stored word."),
        ws == we,
        test_failure("Expecting %d, got %d", we, ws)
    );

    // Reset
    sys_store_word(&sys, 0x00, 0);

    test_check(
        test_print("Store a tetra (%d) in the system's memory.", te),
        sys_store_tetra(&sys, 0x00, te),
        test_failure("Failed to store a tetra...")
    );

    test_check(
        test_print("Load a tetra from the system's memory."),
        sys_load_tetra(&sys, 0x00, &ts),
        test_failure("Failed to load a tetra...")
    );

    test_check(
        test_print("Compare the stored tetra."),
        ts == te,
        test_failure("Expecting %d, got %d", te, ts)
    );

    test_success;
    test_teardown;
    sys_destroy(&sys);
    test_end;
}