#ifndef __RISCV_CACHE_H__
#define __RISCV_CACHE_H__

#include "../../lib/common/include/types.h"
#include <limits.h>

typedef struct {
    bool invalid;
    bool dirty;
    octa addr;
    octa data;
    unsigned int lru_counter;
} data_cache_entry_t;

typedef struct {
    size_t length;
    data_cache_entry_t* base[2];
} data_cache_t;

void data_cache_create(data_cache_t* data_cache, data_cache_entry_t *b0, data_cache_entry_t* b1, size_t length);
bool data_cache_read(data_cache_t* data_cache, octa addr, octa* data);
bool data_cache_write(data_cache_t* data_cache, octa addr, octa data);
bool data_cache_update(data_cache_t* data_cache, octa addr, octa data);

static bool __data_cache_lru(data_cache_t* data_cache, size_t* out)
{
    data_cache_entry_t* it = data_cache->base[0];
    data_cache_entry_t* limit = data_cache->base[0] + data_cache->length - 1;    

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
    data_cache_entry_t* it = data_cache->base[0];
    data_cache_entry_t* limit = data_cache->base[0] + data_cache->length - 1;

    for(;it != limit; it++) 
    {
        if(it->addr == addr && it->invalid == false) {
            *out = it;
            return true;
        }
    }

    return false;
}

void data_cache_create(data_cache_t* data_cache, data_cache_entry_t *b0, data_cache_entry_t* b1, size_t length)
{
    data_cache->base[0] = b0;
    data_cache->base[1] = b1;
    data_cache->length = length;
}

bool data_cache_read(data_cache_t* data_cache, octa addr, octa* data)
{       
    data_cache_entry_t* entry;
    
    if(!__data_cache_access(data_cache, addr, &entry)) 
    {
        size_t idx = 0;
        
        if(__data_cache_lru(data_cache, &idx)) 
        {
            entry = &data_cache->base[1][idx];
            
            entry->addr = addr;
            entry->data = 0;
            entry->invalid = true;
            entry->dirty = false;
            entry->lru_counter = 0;
        }
        return false;
    }

    *data = entry->data;
    entry->lru_counter++;
    return false;
}


bool data_cache_write(data_cache_t* data_cache, octa addr, octa data)
{
    data_cache_entry_t* entry;
    
    if(!__data_cache_access(data_cache, addr, &entry)) 
    {
        size_t idx = 0;
        
        if(__data_cache_lru(data_cache, &idx)) 
        {
            entry = &data_cache->base[1][idx];
            
            entry->addr = addr;
            entry->data = data;
            entry->invalid = false;
            entry->dirty = true;
            entry->lru_counter = 0;
        }
        return false;
    }

    entry->data = data;
    entry->dirty = true;
    return false;
}

bool data_cache_update(data_cache_t* data_cache, octa addr, octa data)
{
    data_cache_entry_t* entry;
    
    if(!__data_cache_access(data_cache, addr, &entry)) 
    {
        size_t idx = 0;
        
        if(__data_cache_lru(data_cache, &idx)) 
        {
            entry = &data_cache->base[1][idx];
            
            entry->addr = addr;
            entry->data = data;
            entry->invalid = false;
            entry->dirty = false;
            entry->lru_counter = 0;
        }
        return false;
    }

    entry->data = data;
    entry->dirty = false;
    entry->invalid = false;
    return false;
}

#endif