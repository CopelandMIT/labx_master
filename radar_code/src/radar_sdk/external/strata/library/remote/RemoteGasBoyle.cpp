/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteGasBoyle.hpp"

#include <common/Buffer.hpp>
#include <common/Serialization.hpp>
#include <universal/components/gas/igasboyle.h>
#include <universal/components/subinterfaces.h>
#include <universal/protocol/protocol_definitions.h>


RemoteGasBoyle::RemoteGasBoyle(IVendorCommands *commands, uint8_t id) :
    m_commands {commands, CMD_COMPONENT, getType(), id, COMPONENT_SUBIF_DEFAULT},
    m_registers(sizeof(uint8_t), commands, getType(), id)
{
}

IRegisters<uint8_t> *RemoteGasBoyle::getIRegisters()
{
    return &m_registers;
}

void RemoteGasBoyle::init_asic()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_INITIALIZE_ASIC);
}

bool RemoteGasBoyle::autoscale()
{
    xensiv_mgs_autoscale_status_t status = XENSIV_MGS_AUTOSCALE_status_IN_PROGRESS;
    uint16_t wLength                     = 1;
    m_commands.vendorRead(FN_GAS_BOYLE_AUTOSCALE, wLength, &status);
    if (status == XENSIV_MGS_AUTOSCALE_status_IN_PROGRESS)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void RemoteGasBoyle::configure(const uint8_t registerList[][2], uint8_t count)
{
    m_commands.vendorWriteMulti(FN_GAS_BOYLE_CONFIGURE, count, registerList);
}

bool RemoteGasBoyle::is_init_done()
{
    bool done;
    m_commands.vendorReadParameters(FN_GAS_BOYLE_IS_INIT_DONE, done);
    return done;
}

int32_t RemoteGasBoyle::trigger_measurement()
{
    int32_t status;
    m_commands.vendorRead(FN_GAS_BOYLE_TRIGGER_MEASUREMENT, sizeof(int32_t), &status);
    return status;
}

void RemoteGasBoyle::is_measurement_done(xensiv_mgs_do_measurement_status_t *status)
{
    m_commands.vendorRead(FN_GAS_BOYLE_IS_MEASUREMENT_DONE, 1, status);
}

void RemoteGasBoyle::get_measurement_data(xensiv_mgs_meas_data_buf_t *data_buf)
{
    const uint16_t wLength = 17;
    m_commands.vendorRead(FN_GAS_BOYLE_GET_MEASUREMENT_DATA, wLength, data_buf);
}

void RemoteGasBoyle::read_measurement_data(xensiv_mgs_meas_data_buf_t *data_buf)
{
    const uint16_t wLength = 17;
    m_commands.vendorRead(FN_GAS_BOYLE_READ_MEASUREMENT_DATA, wLength, data_buf);
}

void RemoteGasBoyle::start_measurement(bool start)
{
    m_commands.vendorWriteParameters(FN_GAS_BOYLE_START_MEASUREMENT, start);
}

void RemoteGasBoyle::get_rrefs(xensiv_mgs_rref_setting_t *rrefs)
{
    const uint16_t wLength = sizeof(xensiv_mgs_rref_setting_t);
    m_commands.vendorRead(FN_GAS_BOYLE_GET_RREFS, wLength, rrefs);
}

void RemoteGasBoyle::service_tick()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_SERVICE_TICK);
}

void RemoteGasBoyle::init()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_INIT);
}

void RemoteGasBoyle::get_driver_version(xensiv_mgs_driver_version_t *data_buf)
{
    const uint16_t wLength = sizeof(xensiv_mgs_driver_version_t);
    m_commands.vendorRead(FN_GAS_BOYLE_GET_DRIVER_VERSION, wLength, data_buf);
}

void RemoteGasBoyle::external_sensor_init()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_EXTERNAL_SENS_INIT);
}

void RemoteGasBoyle::external_sensor_trigger()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_EXTERNAL_SENS_TRIGGER);
}

void RemoteGasBoyle::external_sensor_read()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_EXTERNAL_SENS_READ);
}

void RemoteGasBoyle::external_sensor_get_sht31_data(xensiv_mgs_external_sensor_sht31_data_t *data_buf)
{
    const uint16_t wLength = sizeof(xensiv_mgs_external_sensor_sht31_data_t);
    m_commands.vendorRead(FN_GAS_BOYLE_EXTERNAL_SENS_SHT31_GET_DATA, wLength, data_buf);
}

void RemoteGasBoyle::external_sensor_get_dps368_data(xensiv_mgs_external_sensor_dps368_data_t *data_buf)
{
    const uint16_t wLength = sizeof(xensiv_mgs_external_sensor_dps368_data_t);
    m_commands.vendorRead(FN_GAS_BOYLE_EXTERNAL_SENS_DPS368_GET_DATA, wLength, data_buf);
}

void RemoteGasBoyle::set_heater_context(xensiv_mgs_heater_ctx_t *data_buf)
{
    const uint16_t wLength = sizeof(xensiv_mgs_heater_ctx_t);
    m_commands.vendorWrite(FN_GAS_BOYLE_SET_HEATER_CONTEXT, wLength, data_buf);
}

void RemoteGasBoyle::get_heater_context(xensiv_mgs_heater_ctx_t *data_buf)
{
    const uint16_t wLength = sizeof(xensiv_mgs_heater_ctx_t);
    m_commands.vendorRead(FN_GAS_BOYLE_GET_HEATER_CONTEXT, wLength, data_buf);
}
void RemoteGasBoyle::start_initialization(bool start)
{
    m_commands.vendorWriteParameters(FN_GAS_BOYLE_START_INITIALIZATION, start);
}

bool RemoteGasBoyle::calibrate_heater()
{
    xensiv_mgs_calib_status_t status = XENSIV_MGS_CALIB_STATUS_NOT_INITIALIZED;
    const uint16_t wLength           = sizeof(xensiv_mgs_calib_status_t);

    m_commands.vendorRead(FN_GAS_BOYLE_CALIBRATE_HEATER, wLength, &status);
    if (status == XENSIV_MGS_CALIB_STATUS_DONE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void RemoteGasBoyle::get_calibrate_heater_context(xensiv_mgs_temp_calib_ctx_t *ctx)
{
    const uint16_t wLength = sizeof(xensiv_mgs_temp_calib_ctx_t);
    m_commands.vendorRead(FN_GAS_BOYLE_GET_CALIBRATE_HEATER_CONTEXT, wLength, ctx);
}

void RemoteGasBoyle::get_pid_context(xensiv_mgs_pid_controller_ctx_t *ctx)
{
    const uint16_t wLength = sizeof(xensiv_mgs_pid_controller_ctx_t);
    m_commands.vendorRead(FN_GAS_BOYLE_GET_PID_CONTEXT, wLength, ctx);
}

void RemoteGasBoyle::reset_boyle_device()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_RESET_DEVICE);
}

int32_t RemoteGasBoyle::get_heater_temperature()
{
    int32_t temperature;
    const uint16_t wLength = sizeof(temperature);
    m_commands.vendorRead(FN_GAS_BOYLE_GET_HEATER_TEMPERATURE, wLength, &temperature);
    return temperature;
}

void RemoteGasBoyle::get_device_type(xensiv_mgs_type_t *device_type)
{
    const uint16_t wLength = sizeof(xensiv_mgs_type_t);
    m_commands.vendorRead(FN_GAS_BOYLE_GET_DEVICE_TYPE, wLength, device_type);
}

void RemoteGasBoyle::turn_heater_on()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_SET_TURN_HEATER_ON);
}

void RemoteGasBoyle::turn_heater_off()
{
    m_commands.vendorWrite(FN_GAS_BOYLE_SET_TURN_HEATER_OFF);
}
