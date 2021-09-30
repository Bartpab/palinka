#ifndef __ITF_PROCESSOR_H__
#define __ITF_PROCESSOR_H__

#include "../../lib/common/include/types.h"
#include "./bus.h"
#include "./system_bus.h"

typedef enum {
    PROC_ITF_IDLE,
    PROC_ITF_READ,
    PROC_ITF_WRITE
} processor_itf_cmd_t;

typedef enum {
    PROC_ITF_STATUS_IDLING,
    PROC_ITF_STATUS_READING,
    PROC_ITF_STATUS_WRITING
} processor_itf_status_t;

typedef struct {
    byte hw_interrupt;
    octa mar, mbr;
    byte cmd;
    byte status;
    bool is_master;
} processor_itf_state_t;

typedef struct {
    bus_t* sys_bus;
    processor_itf_state_t state[2];
} processor_itf_t;

void processor_itf_step(processor_itf_t* itf)
{
    processor_itf_state_t* curr = &itf->state[0];     
    processor_itf_state_t* nxt = &itf->state[1];

    byte* cur_control_bus = (byte*) (itf->sys_bus->data[1]);
    octa* cur_address_bus = (octa*) (cur_control_bus + 1);
    octa* cur_data_bus = (octa*) (cur_address_bus + 1);

    byte* nxt_control_bus = (byte*) (itf->sys_bus->data[0]);
    octa* nxt_address_bus = (octa*) (nxt_control_bus + 1);
    octa* nxt_data_bus = (octa*) (nxt_data_bus + 1);

    byte cur_control = *cur_control_bus;
    octa cur_addr = *cur_address_bus;
    octa cur_data = *cur_data_bus;

    if(curr->is_master) 
    {
        if((cur_control & SYSTEM_BUS_READ) && (cur_control & SYSTEM_BUS_ACCEPT)) nxt->mbr = cur_data, *nxt_address_bus = 0, *nxt_control_bus = 0, *nxt_data_bus = 0;
        else if((cur_control & SYSTEM_BUS_WRITE) && (cur_control & SYSTEM_BUS_ACCEPT)) *nxt_address_bus = 0, *nxt_control_bus = 0, *nxt_data_bus = 0;
        else if(cur_control & SYSTEM_BUS_REQUEST) nxt->is_master = false, *nxt_control_bus = SYSTEM_BUS_GRANT;
        else if(curr->cmd == PROC_ITF_WRITE) nxt->cmd = PROC_ITF_IDLE, *nxt_control_bus = SYSTEM_BUS_WRITE, *nxt_address_bus = curr->mar, *nxt_data_bus = curr->mbr;
        else if(curr->cmd == PROC_ITF_READ) nxt->cmd = PROC_ITF_IDLE, *nxt_control_bus = SYSTEM_BUS_READ, *nxt_address_bus = curr->mar, *nxt_data_bus = 0;
    }

    if(cur_control & SYSTEM_BUS_RELEASE) nxt->is_master = true;
    if(cur_control & SYSTEM_BUS_INTERRUPT) nxt->hw_interrupt = (cur_control & SYSTEM_BUS_INTERRUPT) >> 6;
}

 

#endif