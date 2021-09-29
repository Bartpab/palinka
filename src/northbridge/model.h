#ifndef __NORTHBRIDGE_MODEL_H__
#define __NORTHBRIDGE_MODEL_H__

#include "../bus/model.h"

typedef struct {
    struct {
        bus_t* bus;
        unsigned char state;            // 0: Idling, 1: Receiver, 2: Sending
        unsigned char sender, command;  // 0 : NOTHING, 1: STORE, 2: LOAD
    } ram_interface;

    struct {
        bus_t* bus;
        unsigned char state; // 0: Idling, 1: Receiver, 2: Sending
    } cpu_interface;

} northbridge_t;

void cpu_interface_step(system_t* sys, northbridge_t* chip)
{
    // Nothing is plugged...
    if(chip->cpu_interface.bus == 0)
        return;

    switch(chip->cpu_interface.state) 
    {
        case 0: // Idling
            byte* control_bus = (byte*)chip->cpu_interface.bus->data[1];
            octa* address_bus = (octa*) (control_bus + 1);
            octa* data_bus    = (octa*) (address_bus + 1);

            switch(*control_bus) {
                case 1: // Load memory value

                break;
            }
        break;
    }
}


#endif