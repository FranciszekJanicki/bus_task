#include "bus_task.h"
#include "FreeRTOS.h"
#include "bus_manager.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "task.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

static void bus_task_func(void* ctx)
{
    bus_task_ctx_t* task_ctx = (bus_task_ctx_t*)ctx;

    bus_manager_t manager;
    bus_manager_initialize(&manager,
                           task_ctx->action,
                           &task_ctx->config,
                           &task_ctx->interface);

    while (1) {
        bus_manager_process(&manager);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

TaskHandle_t bus_task_create_task(bus_task_ctx_t* task_ctx,
                                  char const* task_name,
                                  StaticTask_t* task_buffer,
                                  UBaseType_t task_priority,
                                  StackType_t* task_stack,
                                  UBaseType_t task_stack_size)
{
    assert(task_ctx != NULL);
    assert(task_name != NULL);
    assert(task_buffer != NULL);
    assert(task_stack != NULL);

    return xTaskCreateStatic(bus_task_func,
                             task_name,
                             task_stack_size,
                             task_ctx,
                             task_priority,
                             task_stack,
                             task_buffer);
}

StreamBufferHandle_t bus_task_create_stream_buffer(
    StaticStreamBuffer_t* stream_buffer,
    UBaseType_t stream_buffer_trigger,
    UBaseType_t stream_buffer_storage_size,
    uint8_t* stream_buffer_storage)
{
    assert(stream_buffer != NULL);
    assert(stream_buffer_storage != NULL);

    return xStreamBufferCreateStatic(stream_buffer_storage_size,
                                     stream_buffer_trigger,
                                     stream_buffer_storage,
                                     stream_buffer);
}

void bus_task_receive_done_callback(TaskHandle_t bus_task)
{
    BaseType_t task_woken = pdFALSE;
    xTaskNotifyFromISR(bus_task,
                       BUS_NOTIFY_RECEIVE_DONE,
                       eSetBits,
                       &task_woken);

    portYIELD_FROM_ISR(task_woken);
}

void bus_task_transmit_done_callback(TaskHandle_t bus_task)
{
    BaseType_t task_woken = pdFALSE;
    xTaskNotifyFromISR(bus_task,
                       BUS_NOTIFY_TRANSMIT_DONE,
                       eSetBits,
                       &task_woken);

    portYIELD_FROM_ISR(task_woken);
}
