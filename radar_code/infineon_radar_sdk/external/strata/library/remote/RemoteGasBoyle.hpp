/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "RemoteRegisters.hpp"
#include "RemoteVendorCommands.hpp"
#include <components/interfaces/IGasBoyle.hpp>


class RemoteGasBoyle :
    public IGasBoyle
{
public:
    RemoteGasBoyle(IVendorCommands *commands, uint8_t id);

    IRegisters<uint8_t> *getIRegisters() override;

    void init_asic() override;
    bool autoscale() override;
    void configure(const uint8_t registerList[][2], uint8_t count) override;
    bool is_init_done() override;
    int32_t trigger_measurement() override;
    void is_measurement_done(xensiv_mgs_do_measurement_status_t *status) override;
    void get_measurement_data(xensiv_mgs_meas_data_buf_t *data_buf) override;
    void read_measurement_data(xensiv_mgs_meas_data_buf_t *data_buf) override;
    void start_measurement(bool start) override;
    void get_rrefs(xensiv_mgs_rref_setting_t *rrefs) override;
    void service_tick() override;
    void init() override;
    void get_driver_version(xensiv_mgs_driver_version_t *data_buf) override;
    void external_sensor_trigger() override;
    void external_sensor_read() override;
    void external_sensor_get_sht31_data(xensiv_mgs_external_sensor_sht31_data_t *data_buf) override;
    void external_sensor_get_dps368_data(xensiv_mgs_external_sensor_dps368_data_t *data_buf) override;
    void external_sensor_init() override;
    void set_heater_context(xensiv_mgs_heater_ctx_t *data_buf) override;
    void get_heater_context(xensiv_mgs_heater_ctx_t *data_buf) override;
    void start_initialization(bool start) override;
    bool calibrate_heater() override;
    void get_calibrate_heater_context(xensiv_mgs_temp_calib_ctx_t *ctx) override;
    void get_pid_context(xensiv_mgs_pid_controller_ctx_t *ctx) override;
    void reset_boyle_device() override;
    int32_t get_heater_temperature() override;
    virtual void get_device_type(xensiv_mgs_type_t *device_type) override;
    void turn_heater_on() override;
    void turn_heater_off() override;

private:
    RemoteVendorCommands m_commands;

    RemoteRegisters<uint8_t> m_registers;
};
