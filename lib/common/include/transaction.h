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
    
    while(it != 0) 
    {
        void* dest = *(void**)(it + 1);
        if(dest >= base && dest < x1) it->invalid = true;
        it = it->next;
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

    transaction->head = transaction->tail = 0;
    transaction->size = 0;
}

#define decl_tst_update_type(type, typename) static void tst_update_ ## typename ## _commit(transaction_log_t* log) \
{\
    uintptr_t base = (uintptr_t)(log + 1); \
    type** dest = (type**)(base); \
    type* value = (type*)(base + sizeof(type*)); \
\
    **dest = *value; \
}\
\
bool tst_update_##typename(transaction_t* transaction, type* dest, type value)\
{\
    if(transaction == 0)\
    {\
        *dest = value;\
        return true;\
    }\
\
    transaction_log_t* log = (transaction_log_t*) tst_new_log(transaction, sizeof(type*) + sizeof(type));\
\
    if(log == NULL)\
        return false;\
\
    uintptr_t base = (uintptr_t)(log + 1);\
    type** __dest = (type**) base;\
    type* __value = (type*)(base + sizeof(type*));\
\
    *__dest = dest;\
    *__value = value;\
\
    log->commit = tst_update_##typename##_commit;\
\
    return true;\
}

decl_tst_update_type(byte, byte)
decl_tst_update_type(word, word)
decl_tst_update_type(tetra, tetra)
decl_tst_update_type(octa, octa)
decl_tst_update_type(unsigned int, uint)
decl_tst_update_type(int, int)
decl_tst_update_type(bool, bool)
decl_tst_update_type(char, char)
decl_tst_update_type(unsigned char, uchar)

#endif