#include "../lib/common/include/testing/utils.h"
#include "../lib/common/include/types.h"
#include "../src/processor/cache.h"

define_test(data_cache, test_print("Data cache")) 
{
    data_cache_t cache;
    allocator_t allocator = GLOBAL_ALLOCATOR;
    data_cache_entry_t* base = pmalloc(&allocator, sizeof(data_cache_entry_t) * 256);
    byte b = 0;

    data_cache_create(&cache, base, 256);

    test_check(
        test_print("Check that we have a cache miss at 0xAB"),
        !data_cache_read(&cache, 0x08, &b, 0),
        test_failure("No cache miss...")
    );

    test_check(
        test_print("Check that we've successfuly written into the cache at 0xAB"),
        data_cache_write(&cache, 0x08, 100, 0),
        test_failure("Failed to write to the cache...")
    );

    test_check(
        test_print("Check that we've successfuly read from the cache at 0xAB"),
        data_cache_read(&cache, 0x08, &b, 0),
        test_failure("Failed to read from the cache...")
    );

    test_check(
        test_print("Check cached value at address 0xAB"), 
        b == 100, 
        test_failure("Expecting 100, got %d", b)
    );

    test_success;
    test_teardown;
    pfree(&allocator, base);
    test_end;
}