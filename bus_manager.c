#include "bus_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static size_t bus_manager_stream_receive_data(bus_manager_t* manager,
                                              uint8_t* data,
                                              size_t data_size)
{
    return xStreamBufferReceive(manager->config.stream_buffer,
                                data,
                                data_size,
                                portMAX_DELAY);
}

static size_t bus_manager_stream_transmit_data(bus_manager_t* manager,
                                               uint8_t const* data,
                                               size_t data_size)
{
    return xStreamBufferSend(manager->config.stream_buffer,
                             data,
                             data_size,
                             portMAX_DELAY);
}

static bus_err_t bus_manager_bus_initialize(bus_manager_t* manager)
{
    return (manager->interface.bus_initialize != NULL)
               ? manager->interface.bus_initialize(manager->interface.bus_user)
               : BUS_ERR_NULL;
}

static bus_err_t bus_manager_bus_deinitialize(bus_manager_t* manager)
{
    return (manager->interface.bus_deinitialize != NULL)
               ? manager->interface.bus_deinitialize(
                     manager->interface.bus_user)
               : BUS_ERR_NULL;
}

static bus_err_t bus_manager_bus_receive_data(bus_manager_t* manager,
                                              uint8_t* data,
                                              size_t data_size)
{
    return (manager->interface.bus_receive_data != NULL)
               ? manager->interface.bus_receive_data(
                     manager->interface.bus_user,
                     data,
                     data_size)
               : BUS_ERR_NULL;
}

static bus_err_t bus_manager_bus_transmit_data(bus_manager_t* manager,
                                               uint8_t const* data,
                                               size_t data_size)
{
    return (manager->interface.bus_transmit_data != NULL)
               ? manager->interface.bus_transmit_data(
                     manager->interface.bus_user,
                     data,
                     data_size)
               : BUS_ERR_NULL;
}

static bool bus_manager_receive_bus_notify(bus_notify_t* notify)
{
    return xTaskNotifyWait(0,
                           BUS_NOTIFY_ALL,
                           (uint32_t*)notify,
                           pdMS_TO_TICKS(10)) == pdPASS;
}

static bus_err_t bus_manager_notify_start_handler(bus_manager_t* manager)
{
    if (manager->is_running) {
        return BUS_ERR_ALREADY_RUNNING;
    }

    manager->is_running = true;

    return BUS_ERR_OK;
}

static bus_err_t bus_manager_notify_stop_handler(bus_manager_t* manager)
{
    if (!manager->is_running) {
        return BUS_ERR_NOT_RUNNING;
    }

    manager->is_running = false;

    return BUS_ERR_OK;
}

static bus_err_t bus_manager_notify_receive_done_handler(bus_manager_t* manager)
{
    if (!manager->is_running) {
        return BUS_ERR_NOT_RUNNING;
    }

    memset(manager->config.bus_buffer, 0, sizeof(manager->config.bus_buffer));

    bus_err_t err =
        bus_manager_bus_receive_data(manager,
                                     manager->config.bus_buffer,
                                     manager->config.bus_buffer_size);
    if (err != BUS_ERR_OK) {
        return err;
    }

    size_t transmitted_size =
        bus_manager_stream_transmit_data(manager,
                                         manager->config.bus_buffer,
                                         manager->config.bus_buffer_size);
    if (transmitted_size == 0U) {
        return BUS_ERR_FAIL;
    }

    return err;
}

static bus_err_t bus_manager_notify_transmit_done_handler(
    bus_manager_t* manager)
{
    if (!manager->is_running) {
        return BUS_ERR_NOT_RUNNING;
    }

    memset(manager->config.bus_buffer,
           0,
           sizeof(manager->config.bus_buffer_size));

    size_t received_size =
        bus_manager_stream_receive_data(manager,
                                        manager->config.bus_buffer,
                                        manager->config.bus_buffer_size);
    if (received_size == 0U) {
        return BUS_ERR_FAIL;
    }

    return bus_manager_bus_transmit_data(manager,
                                         manager->config.bus_buffer,
                                         received_size);
}

static bus_err_t bus_manager_notify_handler(bus_manager_t* manager,
                                            bus_notify_t notify)
{
    if (notify & BUS_NOTIFY_START) {
        return bus_manager_notify_start_handler(manager);
    }
    if (notify & BUS_NOTIFY_START) {
        return bus_manager_notify_stop_handler(manager);
    }
    if (notify & BUS_NOTIFY_TRANSMIT_DONE) {
        return bus_manager_notify_transmit_done_handler(manager);
    }
    if (notify & BUS_NOTIFY_RECEIVE_DONE) {
        return bus_manager_notify_receive_done_handler(manager);
    }

    return BUS_ERR_UNKNOWN_NOTIFY;
}

bus_err_t bus_manager_process(bus_manager_t* manager)
{
    assert(manager != NULL);

    bus_notify_t notify;
    if (bus_manager_receive_bus_notify(&notify)) {
        return bus_manager_notify_handler(manager, notify);
    }

    return BUS_ERR_OK;
}

bus_err_t bus_manager_initialize(bus_manager_t* manager,
                                 bus_action_t action,
                                 bus_config_t const* config,
                                 bus_interface_t const* interface)
{
    assert(manager != NULL);
    assert(config != NULL);
    assert(interface != NULL);

    memset(manager, 0, sizeof(*manager));
    memcpy(&manager->config, config, sizeof(*config));
    memcpy(&manager->interface, interface, sizeof(*interface));

    bus_err_t err = bus_manager_bus_initialize(manager);

    if (action == BUS_ACTION_RECEIVE) {
        err |= bus_manager_notify_receive_done_handler(manager);
    } else if (action == BUS_ACTION_TRANSMIT) {
        err |= bus_manager_notify_transmit_done_handler(manager);
    }

    return err;
}