#ifndef __TRANSACTION_H__
#define __TRANSACTION_H__

#include "./allocator.h"

typedef struct transaction_log_t {
    void (*commit)(struct transaction_log_t* self);
    bool invalid;
    struct transaction_log_t* next;
} transaction_log_t;

typedef struct {
    allocator_t log_allocator;
    void* base;
    size_t size, capacity;
    transaction_log_t *head, *tail;
} transaction_t;

void transaction_create(transaction_t* transaction, allocator_t* log_allocator, size_t capacity)
{
    transaction->log_allocator = allocator_copy(log_allocator);
    transaction->base = pmalloc(log_allocator, capacity);
    transaction->capacity = capacity;
    transaction->size = 0;
    transaction->head = 0;
    transaction->tail = 0;
}

void transaction_destroy(transaction_t* transaction)
{
    if(transaction->base == 0) return;
    
    pfree(&transaction->log_allocator, transaction->base);
    transaction->base = 0;
    transaction->size = transaction->capacity = 0;
    transaction->head = 0;
    transaction->tail = 0;
}

void tst_log_invalid(transaction_t* transaction, void* base, size_t length)
{
    void* x0 = (void*) base;
    void* x1 = (void*) ((uintptr_t)(x0) + length);

    transaction_log_t* it = transaction->head;
    while(it != 0) {
        void* dest = *(void**)(it + 1);
        if(dest >= base && dest < x1) it->invalid = true;
    }
}

static void tst_add_log(transaction_t* transaction, transaction_log_t* log)
{

    
    if(transaction->head == NULL)
        transaction->head = transaction->tail = log;
    else {
        transaction->tail->next = log;
        transaction->tail = log;
    }
}


transaction_log_t* tst_new_log(transaction_t* transaction, size_t payload_size)
{
    if(transaction->base == 0)
        return 0;

    size_t length = sizeof(transaction_log_t) + payload_size;
    
    if(transaction->size + length > transaction->capacity) 
    {
        size_t capacity = transaction->capacity;
        while(capacity < transaction->size + length) {
            if(capacity == 0) capacity = 1;
            else capacity <<= 1;
        }
        transaction_log_t* base = (transaction_log_t*) prealloc(&transaction->log_allocator, transaction->base, capacity);
        if(!base) return NULL;
        transaction->base = base;
        transaction->capacity = capacity;
    }

    char* offset = (char*)transaction->base;
    offset += transaction->size;
    transaction_log_t* log = (transaction_log_t*) offset;
    log->invalid = false;
    log->next = 0;
    tst_add_log(transaction, log);
    transaction->size += length;
    return log;   
}

void tst_commit_log(transaction_log_t* log)
{
    if(!log->invalid)
        log->commit(log);
}

void tst_commit(transaction_t* transaction) {
    transaction_log_t* it = transaction->head;

    while(it != 0) {
        tst_commit_log(it);
        it = it->next;
    }

    transaction->size = 0;
}

static void tst_update_byte_commit(transaction_log_t* log)
{
    byte** dest = (byte**)(log + 1);
    byte* value = (byte*)((char*)(log) + sizeof(byte**));

    **dest = *value;
}

bool tst_update_byte(transaction_t* transaction, byte* dest, byte value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }

    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(byte*) + sizeof(byte));

    if(log == NULL)
        return false;
    
    byte** __dest = (byte**)(log + 1);
    byte* __value = (dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_byte_commit;
    
    return true;
}

static void tst_update_tetra_commit(transaction_log_t* log)
{
    tetra** dest = (tetra**)(log + 1);
    tetra* value = (tetra*)((char*)(log) + sizeof(tetra**));

    **dest = *value;
}

bool tst_update_tetra(transaction_t* transaction, tetra* dest, tetra value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }
    
    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(tetra*) + sizeof(tetra));

    if(log == NULL)
        return false;
    
    tetra** __dest = (tetra**)(log + 1);
    tetra* __value = (dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_tetra_commit;
    
    return true;
}

static void tst_update_octa_commit(transaction_log_t* log)
{
    octa** dest = (octa**)(log + 1);
    octa* value = (octa*)((char*)(log) + sizeof(octa**));

    **dest = *value;
}

bool tst_update_octa(transaction_t* transaction, octa* dest, octa value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }

    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(octa*) + sizeof(octa));

    if(log == NULL)
        return false;
    
    octa** __dest = (octa**)(log + 1);
    octa* __value = (dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_octa_commit;
    
    return true;
}

static void tst_update_bool_commit(transaction_log_t* log)
{
    bool** dest = (bool**)(log + 1);
    bool* value = (bool*)((char*)(log) + sizeof(bool**));

    **dest = *value;
}

bool tst_update_bool(transaction_t* transaction, bool* dest, bool value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }

    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(bool*) + sizeof(bool));

    if(log == NULL)
        return false;
    
    bool** __dest = (bool**)(log + 1);
    bool* __value = (dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_bool_commit;
    
    return true;
}

static void tst_update_char_commit(transaction_log_t* log)
{
    char** dest = (char**)(log + 1);
    char* value = (char*)((char*)(log) + sizeof(char**));

    **dest = *value;
}

bool tst_update_char(transaction_t* transaction, char* dest, char value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }
    
    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(char*) + sizeof(char));

    if(log == NULL)
        return false;
    
    char** __dest = (char**)(log + 1);
    char* __value = (dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_char_commit;
    
    return true;
}

static void tst_update_uchar_commit(transaction_log_t* log)
{
    unsigned char** dest = (unsigned char**)(log + 1);
    unsigned char* value = (unsigned char*)((char*)(log) + sizeof(char**));

    **dest = *value;
}

bool tst_update_uchar(transaction_t* transaction, unsigned char* dest, unsigned char value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }
    
    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(unsigned char*) + sizeof(unsigned char));

    if(log == NULL)
        return false;
    
    unsigned char** __dest = (unsigned char**)(log + 1);
    unsigned char* __value = (unsigned char*)(dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_uchar_commit;
    
    return true;
}

static void tst_update_int_commit(transaction_log_t* log)
{
    int** dest = (int**)(log + 1);
    int* value = (int*)((char*)(log) + sizeof(int**));

    **dest = *value;
}

bool tst_update_int(transaction_t* transaction, int* dest, int value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }
    
    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(int*) + sizeof(int));

    if(log == NULL)
        return false;
    
    int** __dest = (int**)(log + 1);
    int* __value = (dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_int_commit;
    
    return true;
}

static void tst_update_uint_commit(transaction_log_t* log)
{
    unsigned int** dest = (unsigned int**)(log + 1);
    unsigned int* value = (unsigned int*)((char*)(log) + sizeof(unsigned int**));

    **dest = *value;
}

bool tst_update_uint(transaction_t* transaction, unsigned int* dest, unsigned int value)
{
    if(transaction == 0)
    {
        *dest = value;
        return true;
    }
    
    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(unsigned int*) + sizeof(unsigned int));

    if(log == NULL)
        return false;
    
    unsigned int** __dest = (unsigned int**)(log + 1);
    unsigned int* __value = (dest + 1);

    *__dest = dest;
    *__value = value;
    
    log->commit = tst_update_uint_commit;
    
    return true;
}

#endif