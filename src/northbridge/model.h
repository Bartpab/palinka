#ifndef __NORTHBRIDGE_MODEL_H__
#define __NORTHBRIDGE_MODEL_H__

#include "../bus/model.h"


typedef struct {
    octa recv_addr, recv_data;
    byte recv_control;

    octa send_addr, send_data;
    byte send_control;
} fsb_interface_t;

typedef enum {
    HOSTBRIDGE_HOST_CPU,
    HOSTBRIDGE_HOST_MEMORY
} hostbridge_host_t;

typedef struct {
    byte sender, command;
    octa addr, data;
} hostbridge_command_t;

typedef enum {
    HOSTBRIDGE_READ, HOSTBRIDGE_WRITE, HOSTBRIDGE_RESPONSE, HOSTBRIDGE_ACK
} hostbrige_command_type_t;

hostbridge_command_t hostbridge_nop_command = {0, 0, 0, 0};

typedef struct {
    struct {
        system_bus_interface_t io;
        hostbridge_command_t out_queue[32];
        hostbridge_command_t out, in;
    } cpu_controller;

    struct {
        system_bus_interface_t io;
        hostbridge_command_t out_queue[32];
        hostbridge_command_t out, in;
    } memory_controller;

} hostbridge_state_t;

typedef struct {
    bus_t* cpu_bus;
    bus_t* memory_bus;
    hostbridge_state_t state[2];
} hostbridge_t;

void fsb_leader_interface_step(bus_t* bus, system_bus_interface_t* itf)
{
    // Nothing is plugged...
    if(bus == NULL)
        return;
    
    byte* cur_control_bus = (byte*) (bus->data[1]);
    octa* cur_address_bus = (octa*) (cur_control_bus + 1);
    octa* cur_data_bus = (octa*) (cur_address_bus + 1);

    byte* nxt_control_bus = (byte*) (bus->data[0]);
    octa* nxt_address_bus = (octa*) (nxt_control_bus + 1);
    octa* nxt_data_bus = (octa*) (nxt_data_bus + 1);

    byte cur_control = *cur_control_bus;
    octa cur_addr = *cur_address_bus;
    octa cur_data = *cur_data_bus;

    if(cur_control & SYSTEM_BUS_REQUEST) *nxt_control_bus = SYSTEM_BUS_GRANT, nxt_address_bus = 0, nxt_control_bus = 0;
    else if(itf->send_control) *nxt_control_bus = itf->send_control, *nxt_address_bus = itf->send_addr, *nxt_data_bus = itf->send_data;

    // Recv data from the bus
    if(cur_control & SYSTEM_BUS_RESPONSE) itf->recv_addr = cur_addr, itf->recv_data = cur_data, itf->recv_control = cur_control;
}
void fsb_follower_interface_step(bus_t* bus, system_bus_interface_t* itf)
{
    // Nothing is plugged...
    if(bus == NULL)
        return;

    byte* control_bus = (byte*) (bus->data[1]);
    octa* address_bus = (octa*) (control_bus + 1);
    octa* data_bus = (octa*) (address_bus + 1);

    byte* nxt_control_bus = (byte*) (bus->data[0]);
    octa* nxt_address_bus = (octa*) (control_bus + 1);
    octa* nxt_data_bus = (octa*) (address_bus + 1);

    byte curr_control = *control_bus;
    octa curr_addr = *address_bus;
    octa curr_data = *data_bus;

    // Read request from the CPU
    if(curr_control & SYSTEM_BUS_READ || curr_control & SYSTEM_BUS_WRITE) {
        itf->recv_addr = curr_addr, itf->recv_data = curr_data, itf->recv_control = curr_control;
        if(curr_control & SYSTEM_BUS_WRITE)
            itf->send_control = SYSTEM_BUS_ACK;
    }
        

    if(curr_control & SYSTEM_BUS_GRANT) *nxt_control_bus = itf->send_control, *nxt_address_bus = itf->send_addr, *nxt_data_bus = itf->send_data;
    else if(itf->send_control) *nxt_control_bus = SYSTEM_BUS_REQUEST;
}

static inline bool is_nop_command(hostbridge_command_t* cmd)
{
    return cmd->addr == 0 && cmd->command == 0 && cmd->data == 0 && cmd->sender == 0;
}

void hostbridge_cpu_controller(hostbridge_t* bridge)
{
    hostbridge_state_t* curr = &bridge->state[0];
    hostbridge_state_t* nxt  = &bridge->state[1];

    // Received data from the System Bus
    if(curr->cpu_controller.io.recv_control & SYSTEM_BUS_WRITE || curr->cpu_controller.io.recv_control & SYSTEM_BUS_READ) 
    {
        nxt->cpu_controller.in.addr = curr->cpu_controller.io.recv_addr;
        
        if(curr->cpu_controller.io.recv_control & SYSTEM_BUS_WRITE)
            nxt->cpu_controller.in.command = HOSTBRIDGE_WRITE;
        else
            nxt->cpu_controller.in.command = HOSTBRIDGE_READ;

        nxt->cpu_controller.in.data = curr->cpu_controller.io.recv_data;
        nxt->cpu_controller.in.sender = HOSTBRIDGE_HOST_CPU;
    } else {
        nxt->cpu_controller.in = hostbridge_nop_command;
    }
}

void hostbridge_dispatch_step(hostbridge_t* bridge)
{
    hostbridge_state_t* curr = &bridge->state[0];
    
    if(!is_nop_command(&curr->cpu_controller.in)) 
    {
        hostbridge_command_t cmd = curr->cpu_controller.in;
    }
}

#endif