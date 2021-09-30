#ifndef __ITF_SYSTEM_BUS_H__
#define __ITF_SYSTEM_BUS_H__

typedef enum {
    SYSTEM_BUS_READ = 1,
    SYSTEM_BUS_WRITE = 1 << 1,
    SYSTEM_BUS_ACK = 1 << 2,
    SYSTEM_BUS_READY = 1 << 3,
    SYSTEM_BUS_ACCEPT = 1 << 4,
    SYSTEM_BUS_REQUEST = 1 << 5,
    SYSTEM_BUS_GRANT = 1 << 6,
    SYSTEM_BUS_RELEASE = 1 << 7,
    SYSTEM_BUS_INTERRUPT = 0b111 << 9
} system_bus_control_t;

#endif