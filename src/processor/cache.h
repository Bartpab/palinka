#ifndef __RISCV_CACHE_H__
#define __RISCV_CACHE_H__

#include "../../lib/common/include/types.h"
#include "../../lib/common/include/transaction.h"

#include <limits.h>

typedef struct {
    bool invalid;
    bool dirty;
    octa addr;
    byte data;
    unsigned int lru_counter;
} data_cache_entry_t;

typedef struct {
    size_t length;
    data_cache_entry_t* base;
} data_cache_t;

void data_cache_create(data_cache_t* data_cache, data_cache_entry_t *base, size_t length);
bool data_cache_read(data_cache_t* data_cache, octa addr, byte* data, transaction_t* transaction);
bool data_cache_read_word(data_cache_t* data_cache, octa addr, word* data, transaction_t* transaction);
bool data_cache_read_tetra(data_cache_t* data_cache, octa addr, tetra* data, transaction_t* transaction);
bool data_cache_read_octa(data_cache_t* data_cache, octa addr, octa* data, transaction_t* transaction);
bool data_cache_write(data_cache_t* data_cache, octa addr, byte data, transaction_t* transaction);
bool data_cache_write_word(data_cache_t* data_cache, octa addr, word data, transaction_t* transaction);
bool data_cache_write_tetra(data_cache_t* data_cache, octa addr, tetra data, transaction_t* transaction);
bool data_cache_write_octa(data_cache_t* data_cache, octa addr, octa data, transaction_t* transaction);
bool data_cache_update(data_cache_t* data_cache, octa addr, byte data, transaction_t* transaction);

static bool __data_cache_lru(data_cache_t* data_cache, size_t* out)
{
    data_cache_entry_t* it = data_cache->base;
    data_cache_entry_t* limit = data_cache->base + data_cache->length - 1;    

    unsigned int lru_cache = UINT_MAX;
    data_cache_entry_t* candidate = 0;
    size_t idx = 0;
    
    for(unsigned int i = 0;it != limit; it++, i++) 
    {
        if(it->invalid == false && it->dirty == false && it->lru_counter < lru_cache) {
            candidate = it;
            idx = i;
            lru_cache = it->lru_counter;
        }
    }

    if(!candidate)
        return false;
    
    *out = idx;
    return true;
}
static bool __data_cache_access(data_cache_t* data_cache, octa addr, data_cache_entry_t** out)
{
    data_cache_entry_t* it = data_cache->base;
    data_cache_entry_t* limit = data_cache->base + data_cache->length - 1;

    for(;it != limit; it++) 
    {
        if(it->addr == addr && it->invalid == false) {
            *out = it;
            return true;
        }
    }

    return false;
}

void data_cache_create(data_cache_t* data_cache, data_cache_entry_t *base, size_t length)
{
    data_cache->base = base;
    data_cache->length = length;
}

bool data_cache_read(data_cache_t* data_cache, octa addr, byte* data, transaction_t* transaction)
{       
    data_cache_entry_t* entry;
    
    if(!__data_cache_access(data_cache, addr, &entry)) 
    {
        size_t idx = 0;
        
        if(__data_cache_lru(data_cache, &idx)) 
        {
            entry = &data_cache->base[idx];
            
            tst_update_octa(transaction, &entry->addr, addr);
            tst_update_uchar(transaction, &entry->data, 0);
            tst_update_bool(transaction, &entry->invalid, true);
            tst_update_bool(transaction, &entry->dirty, false);
            tst_update_uint(transaction, &entry->lru_counter, 0);
        }
        return false;
    }

    *data = entry->data;
    tst_update_uint(transaction, &entry->lru_counter, entry->lru_counter + 1);
    return false;
}
bool data_cache_read_word(data_cache_t* data_cache, octa addr, word* data, transaction_t* transaction)
{
    byte bytes[2];
    if(!data_cache_read(data_cache, addr, &bytes[0], transaction) || !data_cache_read(data_cache, addr + 1, &bytes[1], transaction))
        return false;
    
    *data = byte_to_word(bytes[1], bytes[0]);
    return true;
}
bool data_cache_read_tetra(data_cache_t* data_cache, octa addr, tetra* data, transaction_t* transaction)
{
    word words[2];
    if(!data_cache_read_word(data_cache, addr, &words[0], transaction) || !data_cache_read_word(data_cache, addr + 2, &words[1], transaction))
        return false;
    
    *data = word_to_tetra(words[1], words[0]);
    return true;
}
bool data_cache_read_octa(data_cache_t* data_cache, octa addr, octa* data, transaction_t* transaction)
{
    tetra tetras[2];
    if(!data_cache_read_tetra(data_cache, addr, &tetras[0], transaction) || !data_cache_read_tetra(data_cache, addr + 4, &tetras[1], transaction))
        return false;
    
    *data = tetra_to_octa(tetras[1], tetras[0]);
    return true;
}

bool data_cache_write(data_cache_t* data_cache, octa addr, byte data, transaction_t* transaction)
{
    data_cache_entry_t* entry;
    
    if(!__data_cache_access(data_cache, addr, &entry)) 
    {
        size_t idx = 0;
        
        if(__data_cache_lru(data_cache, &idx)) 
        {
            entry = &data_cache->base[idx];
            
            tst_update_octa(transaction, &entry->addr, addr);
            tst_update_uchar(transaction, &entry->data, data);
            tst_update_bool(transaction, &entry->invalid, false);
            tst_update_bool(transaction, &entry->dirty, true);
            tst_update_uint(transaction, &entry->lru_counter, 0);
        }
        return false;
    }

    tst_update_uchar(transaction, &entry->data, data);
    tst_update_bool(transaction, &entry->dirty, true);

    return false;
}
bool data_cache_write_word(data_cache_t* data_cache, octa addr, word data, transaction_t* transaction)
{
    byte bytes[2];

    bytes[0] = data & 0xFF;
    bytes[1] = (data >> 8) & 0xFF;

    if(!data_cache_write(data_cache, addr, bytes[0], transaction) || !data_cache_write(data_cache, addr + 1, bytes[1], transaction))
        return false;
    
    return true;
}
bool data_cache_write_tetra(data_cache_t* data_cache, octa addr, tetra data, transaction_t* transaction)
{
    word words[2];
    
    words[0] = data & 0xFFFF;
    words[1] = (data >> 16) & 0xFFFF;

    if(!data_cache_write_word(data_cache, addr, words[0], transaction) || !data_cache_write_word(data_cache, addr + 2, words[1], transaction))
        return false;
    
    return true;
}
bool data_cache_write_octa(data_cache_t* data_cache, octa addr, octa data, transaction_t* transaction)
{
    tetra tetras[2];
    
    tetras[0] = data & 0xFFFFFFFF;
    tetras[1] = (data >> 32) & 0xFFFFFFFF;

    if(!data_cache_write_tetra(data_cache, addr, tetras[0], transaction) || !data_cache_write_tetra(data_cache, addr + 4, tetras[1], transaction))
        return false;
    
    return true;
}

bool data_cache_update(data_cache_t* data_cache, octa addr, byte data, transaction_t* transaction)
{
    data_cache_entry_t* entry;
    
    if(!__data_cache_access(data_cache, addr, &entry)) 
    {
        size_t idx = 0;
        
        if(__data_cache_lru(data_cache, &idx)) 
        {
            entry = &data_cache->base[idx];
            
            tst_update_octa(transaction, &entry->addr, addr);
            tst_update_uchar(transaction, &entry->data, data);
            tst_update_bool(transaction, &entry->invalid, false);
            tst_update_bool(transaction, &entry->dirty, false);
            tst_update_uint(transaction, &entry->lru_counter, 0);
        }
        return false;
    }

    tst_update_uchar(transaction, &entry->data, data);
    tst_update_bool(transaction, &entry->dirty, false);
    tst_update_bool(transaction, &entry->invalid, false);

    return false;
}

#endif