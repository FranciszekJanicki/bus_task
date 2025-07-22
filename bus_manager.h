#ifndef BUS_TASK_BUS_MANAGER_H
#define BUS_TASK_BUS_MANAGER_H

#include "FreeRTOS.h"
#include "bus_config.h"
#include "stream_buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    bus_config_t config;
    bus_interface_t interface;
    bool is_running;
} bus_manager_t;

bus_err_t bus_manager_initialize(bus_manager_t* manager,
                                 bus_config_t const* config,
                                 bus_interface_t const* interface);
bus_err_t bus_manager_process(bus_manager_t* manager);

#endif // BUS_TASK_BUS_MANAGER_H