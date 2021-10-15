#ifndef __ITF_PROCESSOR_H__
#define __ITF_PROCESSOR_H__

#include "../../lib/common/include/types.h"
#include "../../lib/common/include/transaction.h"
#include "../itf/bus.h"
#include "../itf/system_bus.h"

typedef enum processor_itf_cmd_t {
    PROC_ITF_CMD_NOTHING,
    PROC_ITF_CMD_READ,
    PROC_ITF_CMD_WRITE
} processor_itf_cmd_t;

typedef enum processor_itf_status_t {
    PROC_ITF_STATUS_IDLING,
    PROC_ITF_STATUS_STALLING,
    PROC_ITF_STATUS_READ,
    PROC_ITF_STATUS_WRITTEN
} processor_itf_status_t;

typedef enum processor_itf_event_t {
    PROC_ITF_EVENT_WRITTEN,
    PROC_ITF_EVENT_READ,
    PROC_ITF_EVENT_INTERRUPT
} processor_itf_event_t;

typedef struct {
    union {
        struct {
            octa addr;
            byte origin;
        } written;
        
        struct {
            octa addr, data;
            byte origin;
        } read;

        struct {    
            byte code;
        } interrupt;
    };
} processor_itf_event_payload_t;

struct processor_itf_t;
typedef void (*processor_itf_event_handler_t)(void* self, struct processor_itf_t* itf, transaction_t* transaction, processor_itf_event_payload_t payload);

typedef struct processor_itf_t {
    bus_t* sys_bus;

    byte hw_interrupt;
    octa mar, mbr;
    byte origin;
    byte cmd;
    byte status;
    bool leadership;

    struct {
        void* self;
        processor_itf_event_handler_t hdlr;
    } event_handlers[3];

} processor_itf_t;

static unsigned int EVENT_HANDLER_COUNT = 3;

bool processor_itf_write(processor_itf_t* itf, octa addr, octa data, byte origin, transaction_t* transaction)
{
    // Cannot send a command as the controller is not idling
    if(itf->status != PROC_ITF_STATUS_IDLING) return false;

    tst_update_byte(transaction, &itf->cmd, PROC_ITF_CMD_WRITE);
    tst_update_octa(transaction, &itf->mar, addr);
    tst_update_octa(transaction, &itf->mbr, data);
    tst_update_byte(transaction, &itf->origin, origin);

    return true;
}

bool processor_itf_read(processor_itf_t* itf, octa addr, octa data, byte origin, transaction_t* transaction)
{
    // Cannot send a command as the controller is not idling
    if(itf->cmd != PROC_ITF_STATUS_IDLING) return false;

    tst_update_byte(transaction, &itf->cmd, PROC_ITF_CMD_READ);
    tst_update_byte(transaction, &itf->origin, origin);
    tst_update_octa(transaction, &itf->mar, addr);

    return true;
}

void processor_itf_create(processor_itf_t* itf)
{
    itf->sys_bus = 0;
    itf->hw_interrupt = 0;
    itf->mar = itf->mbr = 0;
    itf->cmd = 0;
    itf->status = PROC_ITF_STATUS_IDLING;
    itf->leadership = true;

    for(unsigned char i = 0; i < EVENT_HANDLER_COUNT; i++) 
    {
        itf->event_handlers[i].self = 0;
        itf->event_handlers[i].hdlr = 0;
    }
}

static inline void __processor_itf_launch_event(processor_itf_t* itf, transaction_t* transaction, unsigned int event, processor_itf_event_payload_t payload)
{
    if(event >= EVENT_HANDLER_COUNT) return;
    
    if(itf->event_handlers[event].hdlr) 
    {
        itf->event_handlers[event].hdlr(itf->event_handlers[event].self, itf, transaction, payload);
    }
}

void processor_itf_step(processor_itf_t* itf, transaction_t* transaction)
{
    processor_itf_event_payload_t payload;

    if(itf->hw_interrupt) 
    {
        payload.interrupt.code = itf->hw_interrupt;
        __processor_itf_launch_event(itf, transaction, PROC_ITF_EVENT_INTERRUPT, payload);
        tst_update_byte(transaction, &itf->hw_interrupt, 0);
    }

    switch(itf->status) 
    {
        case PROC_ITF_STATUS_READ:
        payload.read.addr = itf->mar;
        payload.read.data = itf->mbr;
        payload.read.origin = itf->origin;
        __processor_itf_launch_event(itf, transaction, PROC_ITF_EVENT_READ, payload);
        tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_IDLING);
        break;
        case PROC_ITF_STATUS_WRITTEN:
        payload.read.addr = itf->mar;
        payload.read.origin = itf->origin;
        __processor_itf_launch_event(itf, transaction, PROC_ITF_EVENT_WRITTEN, payload);
        tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_IDLING);
        break;
    }

    if(itf->sys_bus == 0) return;

    byte* cur_control_bus = (byte*) (itf->sys_bus->data[1]);
    octa* cur_address_bus = (octa*) (cur_control_bus + 1);
    octa* cur_data_bus    = (octa*) (cur_address_bus + 1);

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

            tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_WRITTEN); // Written
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
        else if(idling && itf->cmd == PROC_ITF_CMD_WRITE) 
        {           
            *nxt_control_bus = SYSTEM_BUS_WRITE; 
            *nxt_address_bus = itf->mar;
            *nxt_data_bus = itf->mbr;

            tst_update_byte(transaction, &itf->cmd, PROC_ITF_CMD_NOTHING); // reset the cmd flag
            tst_update_byte(transaction, &itf->status, PROC_ITF_STATUS_STALLING); // stalling
        }
        // Reading command sent to the ITF controller
        else if(idling && itf->cmd == PROC_ITF_CMD_READ) 
        {           
            *nxt_control_bus = SYSTEM_BUS_READ;
            *nxt_address_bus = itf->mar;
            *nxt_data_bus = 0;

            tst_update_byte(transaction, &itf->cmd, PROC_ITF_CMD_NOTHING); // reset the cmd flag
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