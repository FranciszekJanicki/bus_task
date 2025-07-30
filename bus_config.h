#ifndef BUS_TASK_BUS_NOTIFY_H
#define BUS_TASK_BUS_NOTIFY_H

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
    BUS_ERR_OK,
    BUS_ERR_FAIL,
    BUS_ERR_NULL,
    BUS_ERR_TRANSMIT,
    BUS_ERR_RECEIVE,
    BUS_ERR_NOT_RUNNING,
    BUS_ERR_ALREADY_RUNNING,
    BUS_ERR_UNKNOWN_NOTIFY,
} bus_err_t;

typedef enum {
    BUS_ACTION_TRANSMIT,
    BUS_ACTION_RECEIVE,
    BUS_ACTION_NONE,
} bus_action_t;

typedef enum {
    BUS_NOTIFY_START = ((1 << BUS_ACTION_RECEIVE) | (1 << BUS_ACTION_TRANSMIT) |
                        (1 << BUS_ACTION_NONE)),
    BUS_NOTIFY_STOP = (1 << 3),
    BUS_NOTIFY_TRANSMIT_DONE = (1 << 4),
    BUS_NOTIFY_RECEIVE_DONE = (1 << 5),
    BUS_NOTIFY_ALL = (BUS_NOTIFY_START | BUS_NOTIFY_STOP |
                      BUS_NOTIFY_TRANSMIT_DONE | BUS_NOTIFY_RECEIVE_DONE),
} bus_notify_t;

typedef struct {
    uint8_t* bus_buffer;
    size_t bus_buffer_size;
    StreamBufferHandle_t stream_buffer;
} bus_config_t;

typedef struct {
    void* bus_user;
    bus_err_t (*bus_initialize)(void*);
    bus_err_t (*bus_deinitialize)(void*);
    bus_err_t (*bus_receive_data)(void*, uint8_t*, size_t);
    bus_err_t (*bus_transmit_data)(void*, uint8_t const*, size_t);
} bus_interface_t;

#endif // BUS_TASK_BUS_NOTIFY_H