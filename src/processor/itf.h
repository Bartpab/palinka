#ifndef __ITF_PROCESSOR_H__
#define __ITF_PROCESSOR_H__

#include "../../lib/common/include/types.h"
#include "../itf/bus.h"
#include "../itf/system_bus.h"

typedef enum {
    PROC_ITF_NOTHING,
    PROC_ITF_READ,
    PROC_ITF_WRITE
} processor_itf_cmd_t;

typedef enum {
    PROC_ITF_STATUS_IDLING,
    PROC_ITF_STATUS_STALLING,
    PROC_ITF_STATUS_READ
} processor_itf_status_t;

typedef struct {
    byte hw_interrupt;
    octa mar, mbr;
    byte cmd;
    byte status;
    bool leadership;
} processor_itf_state_t;

typedef struct {
    bus_t* sys_bus;
    processor_itf_state_t state[2];
} processor_itf_t;

void processor_itf_commit_state(processor_itf_t* itf)
{
    itf->state[1] = itf->state[0];
}

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

    if(curr->leadership) 
    {
        bool accept = cur_control & SYSTEM_BUS_ACCEPT;
        bool reading = cur_control & SYSTEM_BUS_READ;
        bool writing = cur_control & SYSTEM_BUS_WRITE;
        bool request_leadership = cur_control & SYSTEM_BUS_REQUEST;

        bool idling = !reading && !writing;

        // Reading \w an accept flag, we can copy the data from the bus to the MBR
        if(reading & accept) 
        {
            nxt->mbr = cur_data;
            
            *nxt_control_bus &= ~SYSTEM_BUS_READY | ~SYSTEM_BUS_READ; // Reset the flag
            *nxt_address_bus = 0; // Reset the address bus
            *nxt_data_bus = 0; // Reset the data bus

            nxt->status = PROC_ITF_STATUS_READ; // Idling
        }
        // Writing \w an accept flag, we can assume the data has been written correctly
        else if(writing & accept) 
        {
            *nxt_control_bus &= ~SYSTEM_BUS_READY | ~SYSTEM_BUS_WRITE; // Reset the flag;
            *nxt_address_bus = 0; // Reset the address bus
            *nxt_data_bus = 0; // Reset the data bus

            nxt->status = PROC_ITF_STATUS_IDLING; // Idling
        }
        // Someone is requesting the system bus master
        else if(idling && request_leadership) 
        {
            nxt->leadership = false; 

            *nxt_control_bus = SYSTEM_BUS_GRANT;
            *nxt_address_bus = 0;
            *nxt_data_bus = 0;

            nxt->status = PROC_ITF_STATUS_STALLING; // stalling
        }
        // Writing command sent to the ITF controller
        else if(idling && curr->cmd == PROC_ITF_WRITE) 
        {           
            *nxt_control_bus = SYSTEM_BUS_WRITE; 
            *nxt_address_bus = curr->mar;
            *nxt_data_bus = curr->mbr;

            nxt->cmd = PROC_ITF_NOTHING; // Reset the cmd flag
            nxt->status = PROC_ITF_STATUS_STALLING; // stalling 
        }
        // Reading command sent to the ITF controller
        else if(idling && curr->cmd == PROC_ITF_READ) 
        {           
            *nxt_control_bus = SYSTEM_BUS_READ;
            *nxt_address_bus = curr->mar;
            *nxt_data_bus = 0;

            nxt->cmd = PROC_ITF_NOTHING; // Reset the cmd flag
            nxt->status = PROC_ITF_STATUS_STALLING; // stalling
        }
    }

    // System bus is released, the processor regains control over the system bus
    if(cur_control & SYSTEM_BUS_RELEASE) 
    {
        nxt->leadership = true;
        nxt->status = PROC_ITF_STATUS_IDLING;
    }

    // An interrupt is sent to the processor
    if(cur_control & SYSTEM_BUS_INTERRUPT) {
        nxt->hw_interrupt = (cur_control & SYSTEM_BUS_INTERRUPT) >> 6;
    }
}

 

#endif