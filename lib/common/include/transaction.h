#ifndef __TRANSACTION_H__
#define __TRANSACTION_H__

#include "./allocator.h"

typedef struct transaction_log_t {
    void (*commit)(struct transaction_log_t* self);
    bool invalid;
    uintptr_t next;
} transaction_log_t;

typedef struct {
    allocator_t log_allocator;
    void* base;
    size_t size, capacity;
    uintptr_t tail;
} transaction_t;

typedef struct {
    transaction_t* transaction;
    transaction_log_t* curr;
    bool started;
} transaction_log_iterator_t;

void transaction_log_iter(transaction_t* transaction, transaction_log_iterator_t* it)
{
    it->curr = 0;
    it->started = 0;
    it->transaction = transaction;
}

bool transaction_log_it_next(transaction_log_iterator_t* it) {
    if(it->started == false)
    {   
        it->curr = (transaction_log_t*)(it->transaction->size > 0 ? it->transaction->base : 0);
        it->started = true;
        
        if(it->transaction->size == 0)
            return false;
        
        return true;
    }

    if(it->curr->next == 0)
        return false;
    else
        it->curr = (transaction_log_t*)((uintptr_t)(it->transaction->base) + it->curr->next);
    
    return true;
}

bool transaction_log_it_get(transaction_log_iterator_t* it, transaction_log_t** out)
{
    if(it->curr != 0) 
    {
        *out = it->curr;
        return true;
    }
    
    return false;
}

void transaction_create(transaction_t* transaction, allocator_t* log_allocator, size_t capacity)
{
    transaction->log_allocator = allocator_copy(log_allocator);
    transaction->base = pmalloc(log_allocator, capacity);
    transaction->capacity = capacity;
    transaction->size = 0;
    transaction->tail = 0;
}

void transaction_destroy(transaction_t* transaction)
{
    if(transaction->base == 0) return;
    
    pfree(&transaction->log_allocator, transaction->base);
    transaction->base = 0;
    transaction->size = transaction->capacity = 0;
    transaction->tail = 0;
}

void tst_log_invalid(transaction_t* transaction, void* base, size_t length)
{
    uintptr_t x0 = (uintptr_t) base;
    uintptr_t x1 = (uintptr_t) (x0 + length);

    transaction_log_iterator_t it;
    transaction_log_iter(transaction, &it);

    transaction_log_t* log;

    while(transaction_log_it_next(&it)) 
    {
        transaction_log_it_get(&it, &log);
        uintptr_t dest = (uintptr_t)(*(void**)(log + 1));
        if(dest >= x0 && dest < x1) log->invalid = true;
    }
}

static void tst_add_log(transaction_t* transaction, transaction_log_t* log, size_t length)
{
    uintptr_t rel = (uintptr_t)(log) - (uintptr_t)(transaction->base);

    if(transaction->size == 0) transaction->tail = rel;
    else {
        transaction_log_t* tail = (transaction_log_t*)(transaction->base + transaction->tail);
        tail->next = rel;
        transaction->tail = rel;
    }

    transaction->size += length;
}

static bool __check_capacity(transaction_t* transaction, size_t payload_size)
{
    size_t length = sizeof(transaction_log_t) + payload_size;
    
    if(transaction->size + length > transaction->capacity) 
    {
        size_t capacity = transaction->capacity;
        
        while(capacity < transaction->size + length) 
        {
            if(capacity == 0) capacity = 1;
            else capacity <<= 1;
        }
        
        transaction_log_t* base = (transaction_log_t*) prealloc(&transaction->log_allocator, transaction->base, capacity);
        
        if(!base) return false;
        
        transaction->base = base;
        transaction->capacity = capacity;
    }

    return true;
}

transaction_log_t* tst_new_log(transaction_t* transaction, size_t payload_size)
{
    if(transaction->base == 0)
        return 0;

    size_t length = sizeof(transaction_log_t) + payload_size;
    
    if(!__check_capacity(transaction, payload_size))
        return NULL;

    transaction_log_t* log = (transaction_log_t*) ((char*)(transaction->base) + transaction->size);
    log->invalid = false;
    log->next = 0;
    tst_add_log(transaction, log, length);
    return log;   
}

void tst_commit_log(transaction_log_t* log)
{
    if(!log->invalid) log->commit(log);
}

void tst_commit(transaction_t* transaction) 
{
    transaction_log_iterator_t it;
    transaction_log_iter(transaction, &it);

    transaction_log_t* log;

    while(transaction_log_it_next(&it)) 
    {
        transaction_log_it_get(&it, &log);
        tst_commit_log(log);
    }

    transaction->tail = 0;
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
    if(memcmp(dest, &value, sizeof(type)) == 0) return true;\
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