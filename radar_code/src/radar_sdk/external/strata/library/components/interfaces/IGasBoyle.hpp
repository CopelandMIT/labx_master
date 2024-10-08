/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/IComponent.hpp>
#include <components/interfaces/IRegisters.hpp>
#include <universal/components/gas.h>


typedef struct
{
    uint8_t major;
    uint8_t minor;
    uint8_t step;
} xensiv_mgs_driver_version_t;

/**
  * This typedef represents mesurement data. This typedef can be cast on uint8_t table
  * which readed registers. All registers will be transposed from 2x uint8_t bytes
  * into uint16_t values exept asic_temp0(due to lack of uint24_t type).
  */
typedef struct
{
    uint16_t rtemp0;
    uint16_t rsens1;
    uint16_t rsens2;
    uint16_t rsens3;
    uint16_t rsens4;
    uint16_t rtemp;
    uint16_t rref_ext;
    uint8_t asic_temp0_lsb;
    uint8_t asic_temp0_csb;
    uint8_t asic_temp0_msb;
} xensiv_mgs_meas_data_buf_t;

/**
 * @brief Reads device ID and set corresponding to that ID device type in context.
 *
 * Enum define possible status of DO_MEASUREMENT state
 *
 */
typedef enum : uint8_t
{
    XENSIV_MGS_DO_MEASUREMENT_STATUS_NOT_INITIALIZED = 0,
    XENSIV_MGS_DO_MEASUREMENT_STATUS_ONGOING,
    XENSIV_MGS_DO_MEASUREMENT_STATUS_DONE
} xensiv_mgs_do_measurement_status_t;

/**
 * That enum describes internal states of the AUTOSCALE state in boyle XXstate machine.
 */
typedef enum : uint8_t
{
    XENSIV_MGS_AUTOSCALE_status_IN_PROGRESS = 0,
    XENSIV_MGS_AUTOSCALE_status_IS_DONE
} xensiv_mgs_autoscale_status_t;
typedef enum : uint8_t
{
    XENSIV_MGS_CALIB_STATUS_NOT_INITIALIZED = 0,
    XENSIV_MGS_CALIB_STATUS_FILLING_BUFFER,
    XENSIV_MGS_CALIB_STATUS_BUFFER_FULL,
    XENSIV_MGS_CALIB_STATUS_DONE
} xensiv_mgs_calib_status_t;

/**
 * This structure holds the rref information.
 */
typedef struct
{
    uint8_t rref1 /**< Referenced resistor 1*/;
    uint8_t rref2 /**< Referenced resistor 2*/;
    uint8_t rref3 /**< Referenced resistor 3*/;
    uint8_t rref4 /**< Referenced resistor 4*/;
} xensiv_mgs_rref_setting_t;

/**
*@brief File structure that represent external sensor data SHT31
*
*/
typedef struct
{
    uint16_t temperature; /**< Temperature from external device */  //int value
    uint8_t humidity; /**< Humidity from external device    */      //int value
} xensiv_mgs_external_sensor_sht31_data_t;

/**
 * @brief File structure that represent external sensor data DPS368
 *
 */
typedef struct
{
    uint16_t temperature; /**< Temperature from external device */  //int value
    uint16_t pressure; /**< Humidity from external device    */     //int value
} xensiv_mgs_external_sensor_dps368_data_t;

/**
 * @brief External sensors error code.
 *
 */
typedef enum
{
    EXTERNAL_SENSOR_OK                = 0,
    EXTERNAL_SENSOR_ERR_CRC_NOT_VALID = -1,
    EXTERNAL_SENSOR_ERR_UNKNOWN       = -2,
    EXTERNAL_SENSOR_ERR_NULL_POINTER  = -3
} xensiv_mgs_external_err_code_t;

/**
 * enum defines the heater modes
 */
typedef enum : uint16_t
{
    XENSIV_MGS_HEATER_MODE_NONE   = 0, /**< initial configuration mode*/
    XENSIV_MGS_HEATER_MODE_SINE   = 1, /**< used to select the sine mode of the heater control*/
    XENSIV_MGS_HEATER_MODE_TOGGLE = 2, /**< used to select the toggle mode of the heater control*/
} xensiv_mgs_heater_mode_t;

/**
 * contains the heater configuration parameters
 */
typedef struct
{
    xensiv_mgs_heater_mode_t mode; /**< defines the heater mode*/
    uint16_t tcr_ppb;              /**< defines the TCR value*/

    uint16_t sine_amplitude; /**< amplitude of sine wave*/

    uint16_t sine_offset; /**< sine offset*/

    uint16_t sine_steps; /**< sine steps, related to period of full sine wave.*/

    uint16_t sine_current_step; /**< sine steps, related to period of full sine wave*/

    bool heater_state; /**< true: heater ON, false: heater OFF*/
} xensiv_mgs_heater_ctx_t;
typedef struct
{
    xensiv_mgs_meas_data_buf_t data;
    xensiv_mgs_external_sensor_sht31_data_t data_sht;
    xensiv_mgs_external_sensor_dps368_data_t data_dps;
    uint16_t value_register_heater_target_temp; /* In units of DegC for Boyle1 and in LSB of ADC units for Boyle2 */
} ifx_Multigas_evalkit_data_t;

/**
 * Structure that holds contains pid context
 *
 */
typedef struct
{
    uint16_t pid_t_offset; /**< 16bit register which compensate the offset error of the
                                   temperature measurement*/

    uint8_t pid_t_scale; /**< 8bit register which transform the ADC code into the temperature
                                   domain*/

    uint16_t res_rtemp_25deg; /**< Rtemp value in 25 degC. Value presented in Ohms */

    uint16_t asic_temp_ave_Q6; /**< Average value of asic temperature in Q6 degC. */

    int32_t asic_temp_var_Q4; /**< Parameter that represent slope of temperature stored in
                                      buffer during calibration*/
} xensiv_mgs_pid_controller_ctx_t;

/**
 * @brief Holds calibration buffer with descriptions.
 */
typedef struct
{
    uint32_t buf[16]; /**< Buffer of calibration values*/
    uint32_t average; /**< Store average value of buffer */

    int32_t m_Q4; /**< Fix Q4 parameter describe slope of collected data.*/
    int32_t c_Q0; /**< Fix Q0 This parameter describe offset of linear function*/
} xensiv_mgs_calib_buffer_t;

/**
 * holds the internal working variables for the temperature calibration module
 */
typedef struct
{
    int16_t a_prime_Q4; /**< Calibration value represented in Fix Q4.
                              Floating point equation:
                              A_prime = A0/knvm3*(knvm1 + alfa*knvm2)*(1+k_cal)

    where:
             alfa = 11.35 */

    int32_t b_prime_Q10; /**< Calibration value represented in Fix Q10.
                             Floating point equation:
                             B_prime = T0-k_cal*knvm2*A0 + T_OS

    where:
             T0 = -273.15
                  T_OS = -0.2*/

    int16_t kcal0prime_Q13; /**< Calibration value represented in Fix Q13.
                                  Floating point equation:
                                  k_cal_prime = alfa/knvm3

    where:
             alfa = 11.35 */


    xensiv_mgs_calib_buffer_t asic_temp; /**< Table that store ADC raw data. Used to calculate
                                              mean average of asic_temp. Mean value would be
                                              used to calculate pid_t_offset and pid_t_scale*/

    xensiv_mgs_calib_buffer_t rtemp; /**< Table that store raw ADC raw data. Used to calculate
                                          mean rtemp. Mean value would be used to calculate
                                          pid_t_offset and pid_t_scale*/

    uint8_t number_of_sample; /**< Parameter describe how many samples occurred from the beginning
                                 of
                                 fill the buffer. By this parameter we can compute position in
                                 buffer. It is useful to store data for example: once every 3
                                 sample */
} xensiv_mgs_temp_calib_ctx_t;

/**
* Contains the device type
*/
typedef enum : uint8_t
{
    XENSIV_MGS_TYPE_INVALID = 0,
    XENSIV_MGS_TYPE_BOYLE_1,
    XENSIV_MGS_TYPE_BOYLE_2
} xensiv_mgs_type_t;


/**
 * \brief Access interface to a radar front end device of the Atr22 family,
 * e.g. 
 */
class IGasBoyle :
    virtual public IComponent
{
public:
    static constexpr uint16_t getType()
    {
        return COMPONENT_TYPE_GAS_BOYLE;
    }

    virtual ~IGasBoyle() = default;

    virtual IRegisters<uint8_t> *getIRegisters()                                                     = 0;
    virtual void init_asic()                                                                         = 0;
    virtual bool autoscale()                                                                         = 0;
    virtual void configure(const uint8_t registerList[][2], uint8_t count)                           = 0;
    virtual bool is_init_done()                                                                      = 0;
    virtual int32_t trigger_measurement()                                                            = 0;
    virtual void is_measurement_done(xensiv_mgs_do_measurement_status_t *status)                     = 0;
    virtual void get_measurement_data(xensiv_mgs_meas_data_buf_t *data_buf)                          = 0;
    virtual void read_measurement_data(xensiv_mgs_meas_data_buf_t *data_buf)                         = 0;
    virtual void start_measurement(bool start)                                                       = 0;
    virtual void get_rrefs(xensiv_mgs_rref_setting_t *rrefs)                                         = 0;
    virtual void service_tick()                                                                      = 0;
    virtual void init()                                                                              = 0;
    virtual void get_driver_version(xensiv_mgs_driver_version_t *data_buf)                           = 0;
    virtual void external_sensor_init()                                                              = 0;
    virtual void external_sensor_trigger()                                                           = 0;
    virtual void external_sensor_read()                                                              = 0;
    virtual void external_sensor_get_sht31_data(xensiv_mgs_external_sensor_sht31_data_t *data_buf)   = 0;
    virtual void external_sensor_get_dps368_data(xensiv_mgs_external_sensor_dps368_data_t *data_buf) = 0;
    virtual void set_heater_context(xensiv_mgs_heater_ctx_t *data_buf)                               = 0;
    virtual void get_heater_context(xensiv_mgs_heater_ctx_t *data_buf)                               = 0;
    virtual void start_initialization(bool start)                                                    = 0;
    virtual bool calibrate_heater()                                                                  = 0;
    virtual void get_calibrate_heater_context(xensiv_mgs_temp_calib_ctx_t *ctx)                      = 0;
    virtual void get_pid_context(xensiv_mgs_pid_controller_ctx_t *ctx)                               = 0;
    virtual void reset_boyle_device()                                                                = 0;
    virtual int32_t get_heater_temperature()                                                         = 0;
    virtual void get_device_type(xensiv_mgs_type_t *device_type)                                     = 0;
    virtual void turn_heater_on()                                                                    = 0;
    virtual void turn_heater_off()                                                                   = 0;
};
