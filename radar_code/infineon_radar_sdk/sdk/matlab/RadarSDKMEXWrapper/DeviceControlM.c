/* ===========================================================================
** Copyright (C) 2021-2022 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/

/*
 * DeviceControlM.c - This the file for wrapping the functionality in the
 * ifxRadar/DeviceControl.c module of the radar sdk.
 *
 * This file wraps the basic device control functions in the radar sdk that
 * include : ifx_avian_create, ifx_avian_get_next_frame, ifx_avian_destroy
 *
 * The calling syntax is:
 *
 *		<out> = DeviceControlM(<function> , <params>, ... )
 *
 *      function        wrap of radar sdk function      params              out
 *
 *      create          ifx_avian_create               device_config       device_handle
 *      get_next_frame  ifx_avian_get_next_frame       device_handle       err_code, num_rx, num_samples_per_chirp, num_chirpts_per_frame, RxFrame
 *      destroy         ifx_avian_destroy              device_handle       VOID
 *
 * e.g.:
 *      device_handle = DeviceControl('create',device_config)
 *      [err_code, frame] = DeviceControl('get_next_frame',device_handle)
 *      DeviceControl('destroy',device_handle)
 *
 * This is a MEX file for MATLAB.
*/

#if !defined(HAVE_OCTAVE) || defined(STD_MEX)
  #include "../ContextWrapper/MWrapHelpers.h"
  #include "../ContextWrapper/Octave5Compatibility.h"
#elif defined(HAVE_OCTAVE)
  #include "ContextWrapper/MWrapHelpers.h"
  #include "ContextWrapper/Octave5Compatibility.h"
#endif

#include "ifxAvian/Avian.h"
#include "ifxBase/Base.h"
#include "ifxCw/DeviceCw.h"

// Note: the arg_pointer_valid is checking if the context argnum pointer argument is valid, if not it raises an exception and terminates the execution
#define device_handle(ctx, argnum)  ((ifx_Avian_Device_t*)arg_pointer_valid((ctx), (argnum)))
#define cw_handle(ctx, argnum)  ((ifx_Device_Cw_t*)arg_pointer_valid((ctx), (argnum)))


static void get_version(WrapperContext *ctx)
{
  const char* version = ifx_sdk_get_version_string();
  // mexPrintf("version: %s\n", version);

  ret_error(ctx, 0);
  ret_string(ctx, 1, version);
}


static void get_version_full(WrapperContext *ctx)
{
  const char* full_version = ifx_sdk_get_version_string_full();

  ret_error(ctx, 0);
  ret_string(ctx, 1, full_version);
}


static void get_list(WrapperContext *ctx)
{
  ifx_List_t* list = ifx_avian_get_list();

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, list);
}


static void get_list_by_sensor_type(WrapperContext *ctx)
{
  const ifx_Radar_Sensor_t sensor_type = (ifx_Radar_Sensor_t) arg_uint32(ctx, 0);
  ifx_List_t* list = ifx_avian_get_list_by_sensor_type(sensor_type);

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, list);
}

static void get_aaf_cutoff_list(WrapperContext* ctx)
{
    ifx_Avian_Device_t* device = device_handle(ctx, 0);
    ifx_List_t* list = ifx_avian_get_aaf_cutoff_list(device);

    mxArray* plhs_1 = mxCreateNumericMatrix(ifx_list_size(list), 1, mxUINT32_CLASS, mxREAL);
    uint32_t* outptr = mxGetData(plhs_1);
    uint32_t* retptr = NULL;

    // populate the single column return matrix with values in the aaf cutoff list
    for (uint32_t i = 0; i < ifx_list_size(list); ++i)
    {   
        retptr = ifx_list_get(list, i);
        *outptr++ = *retptr;
    }

    ret_error(ctx, 0);
    ret(ctx, 1, plhs_1);
    ifx_list_destroy(list);
}

static void get_hp_cutoff_list(WrapperContext* ctx)
{
    ifx_Avian_Device_t* device = device_handle(ctx, 0);
    ifx_List_t* list = ifx_avian_get_hp_cutoff_list(device);

    mxArray* plhs_1 = mxCreateNumericMatrix(ifx_list_size(list), 1, mxUINT32_CLASS, mxREAL);
    uint32_t* outptr = mxGetData(plhs_1);
    uint32_t* retptr = NULL;

    // populate the single column return matrix with values in the hp cutoff list
    for (uint32_t i = 0; i < ifx_list_size(list); ++i)
    {
        retptr = ifx_list_get(list, i);
        *outptr++ = *retptr;
    }

    ret_error(ctx, 0);
    ret(ctx, 1, plhs_1);
    ifx_list_destroy(list);
}

static void create(WrapperContext *ctx)
{
    ifx_Avian_Device_t* device = ifx_avian_create();

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, device);
}


static void create_by_port(WrapperContext *ctx)
{
  const char* port = arg_string(ctx, 0);

  ifx_Avian_Device_t* device = ifx_avian_create_by_port(port);

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, device);
}


static void create_by_uuid(WrapperContext *ctx)
{
  const char* uuid = arg_string(ctx, 0);

  ifx_Avian_Device_t* device = ifx_avian_create_by_uuid(uuid);

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, device);
}

static void set_config(WrapperContext *ctx)
{
    ifx_Avian_Device_t* device = device_handle(ctx, 0);
  const mxArray *mcfg = arg_class_x(ctx, 1, "DeviceConfig");

  ifx_Avian_Config_t config;
  config.rx_mask = pget_uint32(mcfg, 0, "rx_mask");
  config.tx_mask = pget_uint32(mcfg, 0, "tx_mask");
  config.mimo_mode = pget_uint32(mcfg, 0, "mimo_mode");
  config.if_gain_dB = pget_uint32(mcfg, 0, "if_gain_dB");
  config.sample_rate_Hz = pget_uint32(mcfg, 0, "sample_rate_Hz");
  config.tx_power_level = pget_uint32(mcfg, 0, "tx_power_level");
  config.start_frequency_Hz = pget_uint64(mcfg, 0, "start_frequency_Hz");
  config.end_frequency_Hz = pget_uint64(mcfg, 0, "end_frequency_Hz");
  config.num_chirps_per_frame = pget_uint32(mcfg, 0, "num_chirps_per_frame");
  config.num_samples_per_chirp = pget_uint32(mcfg, 0, "num_samples_per_chirp");
  config.chirp_repetition_time_s = pget_float(mcfg, 0, "chirp_repetition_time_s");
  config.frame_repetition_time_s = pget_float(mcfg, 0, "frame_repetition_time_s");
  config.hp_cutoff_Hz = pget_uint32(mcfg, 0, "hp_cutoff_Hz");
  config.aaf_cutoff_Hz = pget_uint32(mcfg, 0, "aaf_cutoff_Hz");

  ifx_avian_set_config(device, &config);

  ret_error(ctx, 0);
}


static void get_config(WrapperContext *ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);
  const mxArray* outTemplate = arg_class_x(ctx, 1, "DeviceConfig");

  mxArray* mcfg = mxDuplicateArray(outTemplate);

  ifx_Avian_Config_t config = { 0 };
  ifx_avian_get_config(device, &config);

  pset_uint32(mcfg, 0, "rx_mask", config.rx_mask);
  pset_uint32(mcfg, 0, "tx_mask", config.tx_mask);
  pset_uint32(mcfg, 0, "mimo_mode", config.mimo_mode);
  pset_uint32(mcfg, 0, "if_gain_dB", config.if_gain_dB);
  pset_uint32(mcfg, 0, "sample_rate_Hz", config.sample_rate_Hz);
  pset_uint32(mcfg, 0, "tx_power_level", config.tx_power_level);
  pset_uint64(mcfg, 0, "start_frequency_Hz", config.start_frequency_Hz);
  pset_uint64(mcfg, 0, "end_frequency_Hz", config.end_frequency_Hz);
  pset_uint32(mcfg, 0, "num_chirps_per_frame", config.num_chirps_per_frame);
  pset_uint32(mcfg, 0, "num_samples_per_chirp", config.num_samples_per_chirp);
  pset_float(mcfg, 0, "chirp_repetition_time_s", config.chirp_repetition_time_s);
  pset_float(mcfg, 0, "frame_repetition_time_s", config.frame_repetition_time_s);
  pset_uint32(mcfg, 0, "hp_cutoff_Hz", config.hp_cutoff_Hz);
  pset_uint32(mcfg, 0, "aaf_cutoff_Hz", config.aaf_cutoff_Hz);

  ret_error(ctx, 0);
  ret(ctx, 1, mcfg);
}


static void get_config_defaults(WrapperContext *ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);
  const mxArray* outTemplate = arg_class_x(ctx, 1, "DeviceConfig");

  mxArray* mcfg = mxDuplicateArray(outTemplate);

  ifx_Avian_Config_t config;
  ifx_avian_get_config_defaults(device, &config);

  pset_uint32(mcfg, 0, "rx_mask", config.rx_mask);
  pset_uint32(mcfg, 0, "tx_mask", config.tx_mask);
  pset_uint32(mcfg, 0, "mimo_mode", config.mimo_mode);
  pset_uint32(mcfg, 0, "if_gain_dB", config.if_gain_dB);
  pset_uint32(mcfg, 0, "sample_rate_Hz", config.sample_rate_Hz);
  pset_uint32(mcfg, 0, "tx_power_level", config.tx_power_level);
  pset_uint64(mcfg, 0, "start_frequency_Hz", config.start_frequency_Hz);
  pset_uint64(mcfg, 0, "end_frequency_Hz", config.end_frequency_Hz);
  pset_uint32(mcfg, 0, "num_chirps_per_frame", config.num_chirps_per_frame);
  pset_uint32(mcfg, 0, "num_samples_per_chirp", config.num_samples_per_chirp);
  pset_float(mcfg, 0, "chirp_repetition_time_s", config.chirp_repetition_time_s);
  pset_float(mcfg, 0, "frame_repetition_time_s", config.frame_repetition_time_s);
  pset_uint32(mcfg, 0, "hp_cutoff_Hz", config.hp_cutoff_Hz);
  pset_uint32(mcfg, 0, "aaf_cutoff_Hz", config.aaf_cutoff_Hz);

  ret_error(ctx, 0);
  ret(ctx, 1, mcfg);
}


static void get_temperature(WrapperContext *ctx)
{
  float temperature = 0.0f;

  ifx_Avian_Device_t* device = device_handle(ctx, 0);

  ifx_avian_get_temperature(device, &temperature);

  ret_error(ctx, 0);
  ret_float(ctx, 1, temperature);
}


static void metrics_to_config(WrapperContext* ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);
  const mxArray *mmetrics = arg_class_x(ctx, 1, "DeviceMetrics");
  const mxArray* outTemplate = arg_class_x(ctx, 2, "DeviceConfig");

  mxArray* mcfg = mxDuplicateArray(outTemplate);

  ifx_Avian_Metrics_t metrics;
  metrics.max_range_m = pget_float(mmetrics, 0, "max_range_m");
  metrics.max_speed_m_s = pget_float(mmetrics, 0, "max_speed_m_s");
  metrics.range_resolution_m = pget_float(mmetrics, 0, "range_resolution_m");
  metrics.center_frequency_Hz = pget_float(mmetrics, 0, "center_frequency_Hz");
  metrics.speed_resolution_m_s = pget_float(mmetrics, 0, "speed_resolution_m_s");

  ifx_Avian_Config_t config;
  ifx_avian_metrics_to_config(device, &metrics, false, &config);

  pset_uint32(mcfg, 0, "rx_mask", config.rx_mask);
  pset_uint32(mcfg, 0, "tx_mask", config.tx_mask);
  pset_uint32(mcfg, 0, "mimo_mode", config.mimo_mode);
  pset_uint32(mcfg, 0, "if_gain_dB", config.if_gain_dB);
  pset_uint32(mcfg, 0, "sample_rate_Hz", config.sample_rate_Hz);
  pset_uint32(mcfg, 0, "tx_power_level", config.tx_power_level);
  pset_uint64(mcfg, 0, "start_frequency_Hz", config.start_frequency_Hz);
  pset_uint64(mcfg, 0, "end_frequency_Hz", config.end_frequency_Hz);
  pset_uint32(mcfg, 0, "num_chirps_per_frame", config.num_chirps_per_frame);
  pset_uint32(mcfg, 0, "num_samples_per_chirp", config.num_samples_per_chirp);
  pset_float(mcfg, 0, "chirp_repetition_time_s", config.chirp_repetition_time_s);
  pset_float(mcfg, 0, "frame_repetition_time_s", config.frame_repetition_time_s);
  pset_uint32(mcfg, 0, "hp_cutoff_Hz", config.hp_cutoff_Hz);
  pset_uint32(mcfg, 0, "aaf_cutoff_Hz", config.aaf_cutoff_Hz);

  ret_error(ctx, 0);
  ret(ctx, 1, mcfg);
}


static void destroy(WrapperContext *ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);

  ifx_avian_destroy(device);

  ret_error(ctx, 0);
}


static void start_acquisition(WrapperContext *ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);

  ifx_avian_start_acquisition(device);

  ret_error(ctx, 0);
}


static void stop_acquisition(WrapperContext *ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);

  ifx_avian_stop_acquisition(device);

  ret_error(ctx, 0);
}


static void get_next_frame(WrapperContext *ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);

  ifx_Cube_R_t* frame_ptr = ifx_avian_get_next_frame(device, NULL);
  const ifx_Error_t err_code = ifx_error_get();
  if (err_code != IFX_OK)
  {
      ret_error(ctx, 0);
      // The MATLAB calling function expects that 5 elements are returned.
      // In order to avoid the "One or more output arguments not assigned during call" exception in MATLAB,
      // we need to return 5 elements here.
      ret_error(ctx, 1);
      ret_error(ctx, 2);
      ret_error(ctx, 3);
      ret_error(ctx, 4);
      return;
  }

  uint32_t num_rx = IFX_CUBE_ROWS(frame_ptr);
  uint32_t num_chirps_per_frame = IFX_CUBE_COLS(frame_ptr);
  uint32_t num_samples_per_chirp = IFX_CUBE_SLICES(frame_ptr);

  mxArray* plhs_0 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for err_code
  mxArray* plhs_1 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_rx
  mxArray* plhs_2 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_samples_per_chirp
  mxArray* plhs_3 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_chirps_per_frame
  mxArray* plhs_4 = mxCreateDoubleMatrix((size_t) num_samples_per_chirp * (size_t) num_chirps_per_frame * (size_t) num_rx, 1, mxREAL); // for frame samples

  // pack err code into plhs_0
  uint32_t *out0 = mxGetData(plhs_0);
  *out0 = err_code;

  if(err_code == IFX_OK)
  {
    // pack num_rx into plhs_1
    uint32_t *out1 = mxGetData(plhs_1);
    *out1 = num_rx;

    // pack num_samples_per_chirp into plhs_2
    uint32_t *out2 = mxGetData(plhs_2);
    *out2 = num_chirps_per_frame;

    // pack num_chirps_per_frame into plhs_3
    uint32_t *out3 = mxGetData(plhs_3);
    *out3 = num_samples_per_chirp;

    // pack frame_ptr data into plhs_4 as one dimension array
    double *out4 = mxGetPr(plhs_4);
    for (uint32_t sample = 0; sample < num_samples_per_chirp; ++sample)
    {
        for (uint32_t chirp = 0; chirp < num_chirps_per_frame; ++chirp)
        {
            for (uint8_t rxidx = 0; rxidx < num_rx; ++rxidx)
            {
                *out4++ = IFX_CUBE_AT(frame_ptr, rxidx, chirp, sample);
            }
        }
    }
  }

  ret(ctx, 0, plhs_0);
  ret(ctx, 1, plhs_1);
  ret(ctx, 2, plhs_2);
  ret(ctx, 3, plhs_3);
  ret(ctx, 4, plhs_4);
  ifx_cube_destroy_r(frame_ptr);
}


static void get_next_frame_timeout(WrapperContext *ctx)
{
  ifx_Avian_Device_t* device = device_handle(ctx, 0);
  uint16_t timeout = arg_uint16(ctx, 1);

  ifx_Cube_R_t* frame_ptr = ifx_avian_get_next_frame_timeout(device, NULL, timeout);
  const ifx_Error_t err_code = ifx_error_get();
  if (err_code != IFX_OK)
  {
      ret_error(ctx, 0);
      // The MATLAB calling function expects that 5 elements are returned.
      // In order to avoid the "One or more output arguments not assigned during call" exception in MATLAB,
      // we need to return 5 elements here.
      ret_error(ctx, 1);
      ret_error(ctx, 2);
      ret_error(ctx, 3);
      ret_error(ctx, 4);
      return;
  }

  uint32_t num_rx = IFX_CUBE_ROWS(frame_ptr);
  uint32_t num_chirps_per_frame = IFX_CUBE_COLS(frame_ptr);
  uint32_t num_samples_per_chirp = IFX_CUBE_SLICES(frame_ptr);

  mxArray* plhs_0 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for err_code
  mxArray* plhs_1 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_rx
  mxArray* plhs_2 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_samples_per_chirp
  mxArray* plhs_3 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_chirps_per_frame
  mxArray* plhs_4 = mxCreateDoubleMatrix((size_t)num_samples_per_chirp * (size_t)num_chirps_per_frame * (size_t)num_rx, 1, mxREAL); // for frame samples

  // pack err code into plhs_0
  uint32_t *out0 = mxGetData(plhs_0);
  *out0 = err_code;

  if(err_code == IFX_OK)
  {
    // pack num_rx into plhs_1
    uint32_t *out1 = mxGetData(plhs_1);
    *out1 = num_rx;

    // pack num_samples_per_chirp into plhs_2
    uint32_t *out2 = mxGetData(plhs_2);
    *out2 = num_chirps_per_frame;

    // pack num_chirps_per_frame into plhs_3
    uint32_t *out3 = mxGetData(plhs_3);
    *out3 = num_samples_per_chirp;

    // pack frame_ptr data into plhs_4 as one dimension array
    double *out4 = mxGetPr(plhs_4);
    for (uint32_t sample = 0; sample < num_samples_per_chirp; ++sample)
    {
        for (uint32_t chirp = 0; chirp < num_chirps_per_frame; ++chirp)
        {
            for (uint8_t rxidx = 0; rxidx < num_rx; ++rxidx)
            {
                *out4++ = IFX_CUBE_AT(frame_ptr, rxidx, chirp, sample);
            }
        }
    }
  }

  ret(ctx, 0, plhs_0);
  ret(ctx, 1, plhs_1);
  ret(ctx, 2, plhs_2);
  ret(ctx, 3, plhs_3);
  ret(ctx, 4, plhs_4);
  ifx_cube_destroy_r(frame_ptr);
}


static void get_register_list_string(WrapperContext* ctx)
{
    ifx_Avian_Device_t* device = device_handle(ctx, 0);
    bool set_trigger_bit = (bool)arg_bool(ctx, 1);

    // allocates memory to c string
    char* register_list_string = ifx_avian_get_register_list_string(device, set_trigger_bit);

    ret_error(ctx, 0);
    ret_string(ctx, 1, (const char*)register_list_string);

    // free the c string
    ifx_mem_free(register_list_string);
}

static void get_board_uuid(WrapperContext *ctx)
{
  const char* uuid = NULL;

  ifx_Avian_Device_t* device = device_handle(ctx, 0);

  uuid = ifx_avian_get_board_uuid(device);

  ret_error(ctx, 0);
  ret_string(ctx, 1, uuid);
}


static void get_sensor_information(WrapperContext *ctx)
{
  const ifx_Radar_Sensor_Info_t* info = NULL;
  ifx_Avian_Device_t* device = device_handle(ctx, 0);
  const mxArray* outTemplate = arg_class_x(ctx, 1, "SensorInfo");
  mxArray* minfo = mxDuplicateArray(outTemplate);

  info = ifx_avian_get_sensor_information(device);

  pset_string(minfo, 0, "description", info->description);
  pset_double(minfo, 0, "min_rf_frequency_Hz", info->min_rf_frequency_Hz);
  pset_double(minfo, 0, "max_rf_frequency_Hz", info->max_rf_frequency_Hz);
  pset_uint8(minfo, 0, "num_tx_antennas", info->num_tx_antennas);
  pset_uint8(minfo, 0, "num_rx_antennas", info->num_rx_antennas);
  pset_uint8(minfo, 0, "max_tx_power", info->max_tx_power);
  pset_uint64(minfo, 0, "device_id", info->device_id);

  ret_error(ctx, 0);
  ret(ctx, 1, minfo);
}

static void get_firmware_information(WrapperContext *ctx)
{
  const ifx_Firmware_Info_t* info = NULL;
  ifx_Avian_Device_t* device = device_handle(ctx, 0);
  const mxArray* outTemplate = arg_class_x(ctx, 1, "FirmwareInfo");
  mxArray* minfo = mxDuplicateArray(outTemplate);

  info = ifx_avian_get_firmware_information(device);

  pset_string(minfo, 0, "description", info->description);
  pset_uint16(minfo, 0, "version_major", info->version_major);
  pset_uint16(minfo, 0, "version_minor", info->version_minor);
  pset_uint16(minfo, 0, "version_build", info->version_build);
  pset_string(minfo, 0, "extended_version", info->extendedVersion);

  ret_error(ctx, 0);
  ret(ctx, 1, minfo);
}

static void cw_create(WrapperContext *ctx)
{
  ifx_Device_Cw_t* new_cw_handle = NULL;

  new_cw_handle = ifx_cw_create();

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, (void*)new_cw_handle);
}

static void cw_destroy(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);

  ifx_cw_destroy(cw_control);

  ret_error(ctx, 0);
}

static void cw_start_signal(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  
  ifx_cw_start_signal(cw_control);

  ret_error(ctx, 0);
}

static void cw_stop_signal(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  
  ifx_cw_stop_signal(cw_control);

  ret_error(ctx, 0);
}

// static void cw_set_rf_frequency_Hz(WrapperContext *ctx)
// {
//   ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
//   uint64_t frequency = arg_uint64(ctx, 1);

//   ifx_cw_set_rf_frequency(cw_control, frequency);

//   ret_error(ctx, 0);
// }

// static void cw_get_rf_frequency_Hz(WrapperContext *ctx)
// {
//   ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);

//   uint64_t frequency = ifx_cw_get_rf_frequency(cw_control);

//   ret_error(ctx, 0);
//   ret_uint64(ctx, 1, frequency);
// }

// static void cw_set_tx_dac_value(WrapperContext *ctx)
// {
//   ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
//   uint32_t dac_value = arg_uint32(ctx, 1);

//   ifx_cw_set_tx_dac_value(cw_control, dac_value);

//   ret_error(ctx, 0);
// }

// static void cw_get_tx_dac_value(WrapperContext *ctx)
// {
//   ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);

//   uint32_t dac_value = ifx_cw_get_tx_dac_value(cw_control);

//   ret_error(ctx, 0);
//   ret_uint32(ctx, 1, dac_value);
// }


static void cw_set_baseband_config(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  const mxArray *mcfg = arg_class_x(ctx, 1, "BasebandConfig");

  ifx_Cw_Baseband_Config_t config;
  //! Todo: if_gain_dB = vga_gain + hp_gain, double check the calculation
  uint32_t vga_gain = pget_uint32(mcfg, 0, "vga_gain");
  uint32_t hp_gain = pget_uint32(mcfg, 0, "hp_gain");
  config.if_gain_dB = vga_gain + hp_gain;
  config.hp_cutoff_Hz = pget_uint32(mcfg, 0, "hp_cutoff_Hz");
  config.lp_cutoff_Hz = pget_uint32(mcfg, 0, "aaf_cutoff_Hz");

  ifx_cw_set_baseband_config(cw_control, &config);

  ret_error(ctx, 0);
}

static void cw_get_baseband_config(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  const mxArray *outTemplate = arg_class_x(ctx, 1, "BasebandConfig");

  mxArray* mcfg = mxDuplicateArray(outTemplate);
  
  ifx_Cw_Baseband_Config_t config = *(ifx_cw_get_baseband_config(cw_control));
  
  pset_uint32(mcfg, 0, "if_gain_dB", config.if_gain_dB);
  pset_uint32(mcfg, 0, "hp_cutoff_Hz", config.hp_cutoff_Hz);
  pset_uint32(mcfg, 0, "aaf_cutoff_Hz", config.lp_cutoff_Hz);

  ret_error(ctx, 0);
  ret(ctx, 1, mcfg);
}

static void cw_set_adc_config(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  const mxArray *mcfg = arg_class_x(ctx, 1, "AdcConfig");

  ifx_Cw_Adc_Config_t config;
  config.additional_subconversions = pget_uint32(mcfg, 0, "additional_subconversions");
  config.sample_and_hold_time_ns = pget_uint32(mcfg, 0, "sample_and_hold_time_ns");
  config.double_msb_time = pget_uint8(mcfg, 0, "double_msb_time");
  config.oversampling_factor = pget_uint32(mcfg, 0, "oversampling_factor");

  ifx_cw_set_adc_config(cw_control, &config);

  ret_error(ctx, 0);
}

static void cw_get_adc_config(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  const mxArray *outTemplate = arg_class_x(ctx, 1, "AdcConfig");

  mxArray* mcfg = mxDuplicateArray(outTemplate);

  ifx_Cw_Adc_Config_t config = *ifx_cw_get_adc_config(cw_control);

  pset_uint32(mcfg, 0, "additional_subconversions", config.additional_subconversions);
  pset_uint32(mcfg, 0, "sample_and_hold_time_ns", config.sample_and_hold_time_ns);
  pset_uint8(mcfg, 0, "double_msb_time", config.double_msb_time);
  pset_uint32(mcfg, 0, "oversampling_factor", config.oversampling_factor);

  ret_error(ctx, 0);
  ret(ctx, 1, mcfg);
}

static void cw_set_test_signal_generator_config(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  const mxArray *mcfg = arg_class_x(ctx, 1, "TestSignalGeneratorConfig");

  ifx_Cw_Test_Signal_Generator_Config_t config;
  config.mode = pget_uint32(mcfg, 0, "mode");
  config.frequency_Hz = pget_float(mcfg, 0, "frequency_Hz");

  ifx_cw_set_test_signal_generator_config(cw_control, &config);

  ret_error(ctx, 0);
}

static void cw_get_test_signal_generator_config(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  const mxArray *outTemplate = arg_class_x(ctx, 1, "TestSignalGeneratorConfig");

  mxArray* mcfg = mxDuplicateArray(outTemplate);
  
  ifx_Cw_Test_Signal_Generator_Config_t config = *(ifx_cw_get_test_signal_generator_config(cw_control));
  
  pset_uint32(mcfg, 0, "mode", config.mode);
  pset_float(mcfg, 0, "frequency_Hz", config.frequency_Hz);

  ret_error(ctx, 0);
  ret(ctx, 1, mcfg);
}

static void cw_measure_temperature(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);

  float temperature = (float) ifx_cw_measure_temperature(cw_control);

  ret_error(ctx, 0);
  ret_float(ctx, 1, temperature);
}

static void cw_measure_tx_power(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);
  uint32_t tx_antenna = arg_uint32(ctx, 1);

  float tx_power = (float) ifx_cw_measure_tx_power(cw_control, tx_antenna);

  ret_error(ctx, 0);
  ret_float(ctx, 1, tx_power);
}

static void cw_capture_frame(WrapperContext *ctx)
{
  ifx_Device_Cw_t* cw_control = cw_handle(ctx, 0);

  ifx_Matrix_R_t* frame = ifx_cw_capture_frame(cw_control, NULL);
  const ifx_Error_t err_code = ifx_error_get();
  if (err_code != IFX_OK)
  {
      ret_error(ctx, 0);
      // The MATLAB calling function expects that 4 elements are returned.
      // In order to avoid the "One or more output arguments not assigned during call" exception in MATLAB,
      // we need to return 4 elements here.
      ret_error(ctx, 1);
      ret_error(ctx, 2);
      ret_error(ctx, 3);
      return;
  }
  uint32_t num_rx = IFX_MAT_ROWS(frame);
  uint32_t num_samples = IFX_MAT_COLS(frame);

  mxArray* plhs_0 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for err_code
  mxArray* plhs_1 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_rx
  mxArray* plhs_2 = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); // for num_samples
  mxArray* plhs_3 = mxCreateDoubleMatrix((size_t)num_samples * (size_t)num_rx, 1, mxREAL); // for frame samples

  // pack err code into plhs_0
  uint32_t *out0 = mxGetData(plhs_0);
  *out0 = err_code;

  if(err_code == IFX_OK)
  {
    // pack num_rx into plhs_1
    uint32_t *out1 = mxGetData(plhs_1);
    *out1 = num_rx;

    // pack num_samples_per_chirp into plhs_2
    uint32_t *out2 = mxGetData(plhs_2);
    *out2 = num_samples;

    // pack frame_ptr data into plhs_4 as one dimension array
    double *out3 = mxGetPr(plhs_3);
    for (uint8_t rxidx = 0; rxidx < IFX_MAT_ROWS(frame); ++rxidx)
    {
        for (uint32_t sample = 0; sample < IFX_MAT_COLS(frame); ++sample)
        {
            *out3++ = IFX_MAT_AT(frame, rxidx, sample);
        }
    }
  }

  ret(ctx, 0, plhs_0);
  ret(ctx, 1, plhs_1);
  ret(ctx, 2, plhs_2);
  ret(ctx, 3, plhs_3);
}


static const CommandDescriptor commands[] = {
    { "get_version", get_version, 2, 0 },
    { "get_version_full", get_version_full, 2, 0 },
    { "get_list", get_list, 2, 0 },
    { "get_list_by_sensor_type", get_list_by_sensor_type, 2, 1 },
    { "create", create, 2, 0 },
    { "create_by_port", create_by_port, 2, 1 },
    { "create_by_uuid", create_by_uuid, 2, 1 },
    { "set_config", set_config, 1, 2 },
    { "get_config", get_config, 2, 2 },
    { "get_config_defaults", get_config_defaults, 2, 2 },
    { "get_temperature", get_temperature, 2, 1 },
    { "metrics_to_config", metrics_to_config, 2, 3 },
    { "destroy", destroy, 1, 1 },
    { "start_acquisition", start_acquisition, 1, 1 },
    { "stop_acquisition", stop_acquisition, 1, 1 },
    { "get_register_list_string", get_register_list_string, 2, 2 },
    { "get_next_frame", get_next_frame, 5, 1 },
    { "get_next_frame_timeout", get_next_frame_timeout, 5, 2 },
    { "get_board_uuid", get_board_uuid, 2, 1 },
    { "get_sensor_information", get_sensor_information, 2, 2 },
    { "get_firmware_information", get_firmware_information, 2, 2 },
    { "get_aaf_cutoff_list", get_aaf_cutoff_list, 2, 1 },
    { "get_hp_cutoff_list", get_hp_cutoff_list, 2, 1 },
    { "cw_create", cw_create, 2, 1 },
    { "cw_destroy", cw_destroy, 1, 1},
    { "cw_start_signal", cw_start_signal, 1, 1 },
    { "cw_stop_signal", cw_stop_signal, 1, 1 },
    { "cw_set_baseband_config", cw_set_baseband_config, 1, 2 },
    { "cw_get_baseband_config", cw_get_baseband_config, 2, 2 },
    { "cw_set_adc_config", cw_set_adc_config, 1, 2 },
    { "cw_get_adc_config", cw_get_adc_config, 2, 2 },
    { "cw_set_test_signal_generator_config", cw_set_test_signal_generator_config, 1, 2 },
    { "cw_get_test_signal_generator_config", cw_get_test_signal_generator_config, 2, 2 },
    { "cw_measure_temperature", cw_measure_temperature, 2, 1 },
    { "cw_measure_tx_power", cw_measure_tx_power, 2, 2 },
    { "cw_capture_frame", cw_capture_frame, 4, 1 },
    { NULL, NULL, 0, 0 }
};

const Wrapper wrapper = {
    "RadarDevice",
    commands
};
