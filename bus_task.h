#ifndef BUS_TASK_BUS_TASK_H
#define BUS_TASK_BUS_TASK_H

#include "FreeRTOS.h"
#include "bus_config.h"
#include "stream_buffer.h"
#include "task.h"
#include <stdint.h>

typedef struct {
  bus_config_t config;
  bus_interface_t interface;
} bus_task_ctx_t;

TaskHandle_t
bus_task_create_task(bus_task_ctx_t *task_ctx, char const *task_name,
                     StaticTask_t *task_buffer, UBaseType_t task_priority,
                     StackType_t *task_stack, UBaseType_t task_stack_size);
StreamBufferHandle_t bus_task_create_stream_buffer(
    StaticStreamBuffer_t *stream_buffer, UBaseType_t stream_buffer_trigger,
    UBaseType_t stream_buffer_storage_size, uint8_t *stream_buffer_storage);

void bus_task_receive_done_callback(TaskHandle_t bus_task);
void bus_task_transmit_done_callback(TaskHandle_t bus_task);

#endif // BUS_TASK_BUS_TASK_H