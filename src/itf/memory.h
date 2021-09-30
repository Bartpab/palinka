#ifndef __ITF_MEMORY_H__
#define __ITF_MEMORY_H__

#include "./bus.h"
#include "./system_bus.h"
#include "../../lib/common/include/types.h"

typedef enum {
    MEM_STATUS_IDLE,
    MEM_STATUS_READING,
    MEM_STATUS_READ,
    MEM_STATUS_WRITING,
    MEM_STATUS_WRITTEN
} memory_itf_status;

typedef struct {
    octa mbr;
    octa mar;
    int status;
} memory_itf_state_t;

typedef struct {
    memory_itf_state_t state[2];
    bus_t* sys_bus;
    size_t base, limit;
} memory_itf_t;

void memory_itf_step(memory_itf_t* itf)
{
    memory_itf_state_t* curr = &itf->state[0];     
    memory_itf_state_t* nxt = &itf->state[1];

    byte* cur_control_bus = (byte*) (itf->sys_bus->data[1]);
    octa* cur_address_bus = (octa*) (cur_control_bus + 1);
    octa* cur_data_bus = (octa*) (cur_address_bus + 1);

    byte* nxt_control_bus = (byte*) (itf->sys_bus->data[0]);
    octa* nxt_address_bus = (octa*) (nxt_control_bus + 1);
    octa* nxt_data_bus = (octa*) (nxt_data_bus + 1);

    byte cur_control = *cur_control_bus;
    octa cur_addr = *cur_address_bus;
    octa cur_data = *cur_data_bus;

    // Not concerned
    if(cur_addr >= itf->base && cur_addr <= itf->limit) return;

    if((cur_control & SYSTEM_BUS_READ) && (cur_control & SYSTEM_BUS_READY)) 
    {
        // The data has been fetched
        if(curr->status == MEM_STATUS_READ) 
        {
            *nxt_control_bus = SYSTEM_BUS_READ | SYSTEM_BUS_ACCEPT;
            *nxt_data_bus = curr->mbr;
            nxt->status = MEM_STATUS_IDLE;
        } 
        
        if(curr->status == MEM_STATUS_IDLE) 
        {
            nxt->status = MEM_STATUS_READING;
            nxt->mar = cur_addr;
        }
    }

    if((cur_control & SYSTEM_BUS_WRITE) && (cur_control & SYSTEM_BUS_READY)) 
    {
        if(curr->status == MEM_STATUS_WRITTEN) 
        {
            *nxt_control_bus = SYSTEM_BUS_WRITE | SYSTEM_BUS_ACCEPT;
            nxt->status = MEM_STATUS_IDLE;
        }

        if(curr->status == MEM_STATUS_IDLE) 
        {
            nxt->status = MEM_STATUS_WRITING;
            nxt->mar = cur_addr;
            nxt->mbr = cur_data;
        }
    }
}

#endif