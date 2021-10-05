#ifndef __ITF_PROCESSOR_H__
#define __ITF_PROCESSOR_H__

#include "../../lib/common/include/types.h"
#include "../../lib/common/include/transaction.h"
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
    bus_t* sys_bus;

    byte hw_interrupt;
    octa mar, mbr;
    byte cmd;
    byte status;
    bool leadership;
} processor_itf_t;

void processor_itf_step(processor_itf_t* itf, transaction_t* transaction)
{
    if(itf->sys_bus == 0) return;

    byte* cur_control_bus = (byte*) (itf->sys_bus->data[1]);
    octa* cur_address_bus = (octa*) (cur_control_bus + 1);
    octa* cur_data_bus = (octa*) (cur_address_bus + 1);

    byte* nxt_control_bus = (byte*) (itf->sys_bus->data[0]);
    octa* nxt_address_bus = (octa*) (nxt_control_bus + 1);
    octa* nxt_data_bus = (octa*) (nxt_address_bus + 1);

    byte cur_control = *cur_control_bus;
    //octa cur_addr = *cur_address_bus;
    octa cur_data = *cur_data_bus;

    if(itf->leadership) 
    {
        bool accept = cur_control & SYSTEM_BUS_ACCEPT;
        bool reading = cur_control & SYSTEM_BUS_READ;
        bool writing = cur_control & SYSTEM_BUS_WRITE;
        bool request_leadership = cur_control & SYSTEM_BUS_REQUEST;

        bool idling = !reading && !writing;

        // Reading \w an accept flag, we can copy the data from the bus to the MBR
        if(reading & accept) 
        {
            tst_update_octa(transaction, &itf->mbr, cur_data);
            
            *nxt_control_bus &= ~SYSTEM_BUS_READY | ~SYSTEM_BUS_READ; // Reset the flag
            *nxt_address_bus = 0; // Reset the address bus
            *nxt_data_bus = 0; // Reset the data bus

            tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_READ); // Read
        }
        // Writing \w an accept flag, we can assume the data has been written correctly
        else if(writing & accept) 
        {
            *nxt_control_bus &= ~SYSTEM_BUS_READY | ~SYSTEM_BUS_WRITE; // Reset the flag;
            *nxt_address_bus = 0; // Reset the address bus
            *nxt_data_bus = 0; // Reset the data bus

            tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_IDLING); // Idling
        }
        // Someone is requesting the system bus master
        else if(idling && request_leadership) 
        {
            tst_update_bool(transaction, &itf->leadership, false);

            *nxt_control_bus = SYSTEM_BUS_GRANT;
            *nxt_address_bus = 0;
            *nxt_data_bus = 0;

            tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_STALLING); // Stalling
        }
        // Writing command sent to the ITF controller
        else if(idling && itf->cmd == PROC_ITF_WRITE) 
        {           
            *nxt_control_bus = SYSTEM_BUS_WRITE; 
            *nxt_address_bus = itf->mar;
            *nxt_data_bus = itf->mbr;

            tst_update_byte(transaction, &itf->cmd, PROC_ITF_NOTHING); // reset the cmd flag
            tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_STALLING); // stalling
        }
        // Reading command sent to the ITF controller
        else if(idling && itf->cmd == PROC_ITF_READ) 
        {           
            *nxt_control_bus = SYSTEM_BUS_READ;
            *nxt_address_bus = itf->mar;
            *nxt_data_bus = 0;

            tst_update_byte(transaction, &itf->cmd, PROC_ITF_NOTHING); // reset the cmd flag
            tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_STALLING); // stalling
        }
    }

    // System bus is released, the processor regains control over the system bus
    if(cur_control & SYSTEM_BUS_RELEASE) 
    {
        tst_update_bool(transaction, &itf->leadership, true);
        tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_IDLING); // stalling
    }

    // An interrupt is sent to the processor
    if(cur_control & SYSTEM_BUS_INTERRUPT) {
        tst_update_byte(transaction, &itf->hw_interrupt, (cur_control & SYSTEM_BUS_INTERRUPT) >> 6);
    }
}

 

#endif