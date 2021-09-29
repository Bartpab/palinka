#ifndef __BUS_CORE_H__
#define __BUS_CORE_H__

#include "../system.h"
#include "./bus.h"

void bus_step(system_t* sys);

void __bus_init(system_t* sys)
{
  bus_t* bus = __get_bus(sys);

  char* data_0 = (char*)(bus + 1);
  char* data_1 = data_0 + length;

  bus->data[0] = data_0;
  bus->data[1] = data_1;
  bus->length = length;

  // Set v-table
  sys->step = bus_step;
}

system_t* bus_new(size_t length, allocator_t* allocator)
{
  system_t* sys = (system_t*) pmalloc(
    allocator, 
    sizeof(system_t) + sizeof(bus_t) + length * 2
  );
  
  if(!sys)
    return NULL;

  __sys_init(sys, allocator);
  __bus_init(sys);

  return sys;
}

void bus_step(system_t* sys)
{
    bus_t* bus = __get_bus(sys);
    memcpy(&bus->data[1], &bus->data[0], bus->length);
}

#endif