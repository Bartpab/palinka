#include "../include/testing/utils.h"
#include "../include/transaction.h"

define_test(transaction, test_print("Transaction"))
{
    allocator_t allocator = GLOBAL_ALLOCATOR;
    transaction_t transaction;
    transaction_create(&transaction, &allocator, 1);
    byte bte = 0;
    tst_update_byte(&transaction, &bte, 10);

    test_check(
        test_print("Check that the initial value of the byte is 0."),
        bte == 0,
        test_failure("Expecing 0, got %d", bte)
    );

    tst_commit(&transaction);

    test_check(
        test_print("Check that the new value of the byte after commit is 10."),
        bte == 10,
        test_failure("Expecing 10, got %d", bte)
    );

    test_success;
    test_teardown;
    transaction_destroy(&transaction);
    test_end;
}