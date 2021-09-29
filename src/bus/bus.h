#ifndef __BUS_BUS_H__
#define __BUS_BUS_H__

#include <stddef.h>

typedef struct {
    char    *data[2];
    size_t  length;
} bus_t;

bus_t* __get_bus(system_t* sys)
{
  return (bus_t*) (sys + 1);
}

#endif