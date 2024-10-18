/* ===========================================================================
** Copyright (C) 2021-2023 Infineon Technologies AG
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

#include "ifxMimose/DeviceMimose.h"
#include "ifxBase/Base.h"

// Note: the arg_pointer_valid is checking if the context argnum pointer argument is valid, if not it raises an exception and terminates the execution
#define device_handle(ctx, argnum)  ((ifx_Avian_Device_t*)arg_pointer_valid((ctx), (argnum)))
#define mimose_handle(ctx, argnum)  ((ifx_Mimose_Device_t*)arg_pointer_valid((ctx), (argnum)))
#define cw_handle(ctx, argnum)  ((ifx_Device_Cw_t*)arg_pointer_valid((ctx), (argnum)))

/*
==============================================================================
   1. LOCAL FUNCTIONS
==============================================================================
*/

static void get_config_routine(WrapperContext* ctx, ifx_Mimose_Config_t* config)
{
    const mxArray* outTemplate_PC = arg_class_x(ctx, 1, "PulseConfig");
    const mxArray* outTemplate_FC = arg_class_x(ctx, 2, "FrameConfig");
    const mxArray* outTemplate_AC = arg_class_x(ctx, 3, "AFC_Config");
    const mxArray* outTemplate_CC = arg_class_x(ctx, 4, "ClockConfig");

    mxArray* mcfg_PC0 = mxDuplicateArray(outTemplate_PC);
    mxArray* mcfg_PC1 = mxDuplicateArray(outTemplate_PC);
    mxArray* mcfg_PC2 = mxDuplicateArray(outTemplate_PC);
    mxArray* mcfg_PC3 = mxDuplicateArray(outTemplate_PC);

    mxArray* mcfg_FC0 = mxDuplicateArray(outTemplate_FC);
    mxArray* mcfg_FC1 = mxDuplicateArray(outTemplate_FC);

    mxArray* mcfg_AC = mxDuplicateArray(outTemplate_AC);
    mxArray* mcfg_CC = mxDuplicateArray(outTemplate_CC);

    pset_uint32(mcfg_PC0, 0, "channel", config->pulse_config[0].channel);
    pset_uint8(mcfg_PC0, 0, "tx_power_level", config->pulse_config[0].tx_power_level);
    pset_uint32(mcfg_PC0, 0, "abb_gain_type", config->pulse_config[0].abb_gain_type);
    pset_uint32(mcfg_PC0, 0, "aoc_mode", config->pulse_config[0].aoc_mode);
    ret(ctx, 1, mcfg_PC0);

    pset_uint32(mcfg_PC1, 0, "channel", config->pulse_config[1].channel);
    pset_uint8(mcfg_PC1, 0, "tx_power_level", config->pulse_config[1].tx_power_level);
    pset_uint32(mcfg_PC1, 0, "abb_gain_type", config->pulse_config[1].abb_gain_type);
    pset_uint32(mcfg_PC1, 0, "aoc_mode", config->pulse_config[1].aoc_mode);
    ret(ctx, 2, mcfg_PC1);

    pset_uint32(mcfg_PC2, 0, "channel", config->pulse_config[2].channel);
    pset_uint8(mcfg_PC2, 0, "tx_power_level", config->pulse_config[2].tx_power_level);
    pset_uint32(mcfg_PC2, 0, "abb_gain_type", config->pulse_config[2].abb_gain_type);
    pset_uint32(mcfg_PC2, 0, "aoc_mode", config->pulse_config[2].aoc_mode);
    ret(ctx, 3, mcfg_PC2);

    pset_uint32(mcfg_PC3, 0, "channel", config->pulse_config[3].channel);
    pset_uint8(mcfg_PC3, 0, "tx_power_level", config->pulse_config[3].tx_power_level);
    pset_uint32(mcfg_PC3, 0, "abb_gain_type", config->pulse_config[3].abb_gain_type);
    pset_uint32(mcfg_PC3, 0, "aoc_mode", config->pulse_config[3].aoc_mode);
    ret(ctx, 4, mcfg_PC3);

    pset_float(mcfg_FC0, 0, "frame_repetition_time_s", config->frame_config[0].frame_repetition_time_s);
    pset_float(mcfg_FC0, 0, "pulse_repetition_time_s", config->frame_config[0].pulse_repetition_time_s);
    pset_bool(mcfg_FC0, 0, "selected_pulse_config_0", config->frame_config[0].selected_pulse_configs[0]);
    pset_bool(mcfg_FC0, 0, "selected_pulse_config_1", config->frame_config[0].selected_pulse_configs[1]);
    pset_bool(mcfg_FC0, 0, "selected_pulse_config_2", config->frame_config[0].selected_pulse_configs[2]);
    pset_bool(mcfg_FC0, 0, "selected_pulse_config_3", config->frame_config[0].selected_pulse_configs[3]);
    pset_uint32(mcfg_FC0, 0, "num_samples", config->frame_config[0].num_samples);
    ret(ctx, 5, mcfg_FC0);

    pset_float(mcfg_FC1, 0, "frame_repetition_time_s", config->frame_config[1].frame_repetition_time_s);
    pset_float(mcfg_FC1, 0, "pulse_repetition_time_s", config->frame_config[1].pulse_repetition_time_s);
    pset_bool(mcfg_FC1, 0, "selected_pulse_config_0", config->frame_config[1].selected_pulse_configs[0]);
    pset_bool(mcfg_FC1, 0, "selected_pulse_config_1", config->frame_config[1].selected_pulse_configs[1]);
    pset_bool(mcfg_FC1, 0, "selected_pulse_config_2", config->frame_config[1].selected_pulse_configs[2]);
    pset_bool(mcfg_FC1, 0, "selected_pulse_config_3", config->frame_config[1].selected_pulse_configs[3]);
    pset_uint32(mcfg_FC1, 0, "num_samples", config->frame_config[1].num_samples);
    ret(ctx, 6, mcfg_FC1);

    pset_uint32(mcfg_AC, 0, "band", config->afc_config.band);
    pset_uint64(mcfg_AC, 0, "rf_center_frequency_Hz", config->afc_config.rf_center_frequency_Hz);
    pset_uint32(mcfg_AC, 0, "afc_duration_ct", config->afc_config.afc_duration_ct);
    pset_uint32(mcfg_AC, 0, "afc_threshold_course", config->afc_config.afc_threshold_course);
    pset_uint32(mcfg_AC, 0, "afc_threshold_fine", config->afc_config.afc_threshold_fine);
    pset_uint8(mcfg_AC, 0, "afc_period", config->afc_config.afc_period);
    pset_uint32(mcfg_AC, 0, "afc_repeat_count", config->afc_config.afc_repeat_count);

    ret(ctx, 7, mcfg_AC);

    pset_uint32(mcfg_CC, 0, "reference_clock_Hz", config->clock_config.reference_clock_Hz);
    pset_uint32(mcfg_CC, 0, "system_clock_Hz", config->clock_config.system_clock_Hz);
    pset_bool(mcfg_CC, 0, "rc_clock_enabled", config->clock_config.rc_clock_enabled);
    pset_uint32(mcfg_CC, 0, "hf_on_time_usec", config->clock_config.hf_on_time_usec);
    pset_uint16(mcfg_CC, 0, "system_clock_divider", config->clock_config.system_clock_divider);
    pset_bool(mcfg_CC, 0, "system_clock_div_flex", config->clock_config.system_clock_div_flex);
    pset_bool(mcfg_CC, 0, "sys_clk_to_i2c", config->clock_config.sys_clk_to_i2c);

    ret(ctx, 8, mcfg_CC);
}

static void next_frame_routine(WrapperContext* ctx, ifx_Cube_C_t* frame_ptr)
{
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
    mxArray* plhs_4 = mxCreateDoubleMatrix((size_t)num_samples_per_chirp *
        (size_t)num_chirps_per_frame *
        (size_t)num_rx *
        2,  // complex samples
        1, mxREAL); // for frame samples

// pack err code into plhs_0
    uint32_t* out0 = mxGetData(plhs_0);
    *out0 = err_code;

    if (err_code == IFX_OK)
    {
        // pack num_rx into plhs_1
        uint32_t* out1 = mxGetData(plhs_1);
        *out1 = num_rx;

        // pack num_samples_per_chirp into plhs_2
        uint32_t* out2 = mxGetData(plhs_2);
        *out2 = num_chirps_per_frame;

        // pack num_chirps_per_frame into plhs_3
        uint32_t* out3 = mxGetData(plhs_3);
        *out3 = num_samples_per_chirp;

        // pack frame_ptr data into plhs_4 as one dimension array
        double* out4 = mxGetPr(plhs_4);
        for (uint32_t sample = 0; sample < num_samples_per_chirp; ++sample)
        {
            for (uint32_t chirp = 0; chirp < num_chirps_per_frame; ++chirp)
            {
                for (uint8_t rxidx = 0; rxidx < num_rx; ++rxidx)
                {
                    *out4++ = IFX_COMPLEX_REAL(IFX_CUBE_AT(frame_ptr, rxidx, chirp, sample));
                    *out4++ = IFX_COMPLEX_IMAG(IFX_CUBE_AT(frame_ptr, rxidx, chirp, sample));
                }
            }
        }
    }

    ret(ctx, 0, plhs_0);
    ret(ctx, 1, plhs_1);
    ret(ctx, 2, plhs_2);
    ret(ctx, 3, plhs_3);
    ret(ctx, 4, plhs_4);
}

/*
==============================================================================
   2. MEX API FUNCTIONS
==============================================================================
*/

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
  ifx_List_t* list = ifx_mimose_get_list();

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, list);
}


static void create(WrapperContext *ctx)
{
    ifx_Mimose_Device_t* device = ifx_mimose_create();

    ret_error(ctx, 0);
    ret_pointer(ctx, 1, device);
}


static void create_by_uuid(WrapperContext *ctx)
{
  const char* uuid = arg_string(ctx, 0);

  ifx_Mimose_Device_t* device = ifx_mimose_create_by_uuid(uuid);

  ret_error(ctx, 0);
  ret_pointer(ctx, 1, device);
}

static void set_config(WrapperContext *ctx)
{
  ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
  const mxArray* mcfg;

  ifx_Mimose_Config_t config;
  ifx_mimose_get_config_defaults(device, &config);
  
  for (uint32_t PulseConfigIdx = 0; PulseConfigIdx < 3; ++PulseConfigIdx)
  {
      mcfg = arg_class_x(ctx, 1 + PulseConfigIdx, "PulseConfig");
      config.pulse_config[PulseConfigIdx].channel = pget_uint32(mcfg, 0, "channel");
      config.pulse_config[PulseConfigIdx].tx_power_level = pget_uint8(mcfg, 0, "tx_power_level");
      config.pulse_config[PulseConfigIdx].abb_gain_type = pget_uint32(mcfg, 0, "abb_gain_type");
      config.pulse_config[PulseConfigIdx].aoc_mode = pget_uint32(mcfg, 0, "aoc_mode");
  }

  for (uint32_t FrameConfigIdx = 0; FrameConfigIdx < 2; ++FrameConfigIdx)
  {
      mcfg = arg_class_x(ctx, 5 + FrameConfigIdx, "FrameConfig");
      config.frame_config[FrameConfigIdx].frame_repetition_time_s = pget_float(mcfg, 0, "frame_repetition_time_s");
      config.frame_config[FrameConfigIdx].pulse_repetition_time_s = pget_float(mcfg, 0, "pulse_repetition_time_s");
      config.frame_config[FrameConfigIdx].selected_pulse_configs[0] = pget_bool(mcfg, 0, "selected_pulse_config_0");
      config.frame_config[FrameConfigIdx].selected_pulse_configs[1] = pget_bool(mcfg, 0, "selected_pulse_config_1");
      config.frame_config[FrameConfigIdx].selected_pulse_configs[2] = pget_bool(mcfg, 0, "selected_pulse_config_2");
      config.frame_config[FrameConfigIdx].selected_pulse_configs[3] = pget_bool(mcfg, 0, "selected_pulse_config_3");
      config.frame_config[FrameConfigIdx].num_samples = pget_uint32(mcfg, 0, "num_samples");
  }
  mcfg = arg_class_x(ctx, 7, "AFC_Config");
  config.afc_config.band = pget_uint32(mcfg, 0, "band");
  config.afc_config.rf_center_frequency_Hz = pget_uint64(mcfg, 0, "rf_center_frequency_Hz");
  config.afc_config.afc_duration_ct = pget_uint32(mcfg, 0, "afc_duration_ct");
  config.afc_config.afc_threshold_course = pget_uint32(mcfg, 0, "afc_threshold_course");
  config.afc_config.afc_threshold_fine = pget_uint32(mcfg, 0, "afc_threshold_fine");
  config.afc_config.afc_period = pget_uint8(mcfg, 0, "afc_period");
  config.afc_config.afc_repeat_count = pget_uint32(mcfg, 0, "afc_repeat_count");

  mcfg = arg_class_x(ctx, 8, "ClockConfig");
  config.clock_config.reference_clock_Hz = pget_uint32(mcfg, 0, "reference_clock_Hz");
  config.clock_config.system_clock_Hz = pget_uint32(mcfg, 0, "system_clock_Hz");
  config.clock_config.rc_clock_enabled = pget_bool(mcfg, 0, "rc_clock_enabled");
  config.clock_config.hf_on_time_usec  = pget_uint32(mcfg, 0, "hf_on_time_usec");
  config.clock_config.system_clock_divider = pget_uint16(mcfg, 0, "system_clock_divider");
  config.clock_config.system_clock_div_flex = pget_bool(mcfg, 0, "system_clock_div_flex");
  config.clock_config.sys_clk_to_i2c = pget_bool(mcfg, 0, "sys_clk_to_i2c");

  ifx_mimose_set_config(device, &config);

  ret_error(ctx, 0);
}

static void get_config(WrapperContext *ctx)
{
  ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
  ifx_Mimose_Config_t config = { 0 };
  ifx_mimose_get_config(device, &config);
  ret_error(ctx, 0);
  get_config_routine(ctx, &config);
}


static void get_config_defaults(WrapperContext *ctx)
{
    ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
    ifx_Mimose_Config_t config;
    ifx_mimose_get_config_defaults(device, &config);
    ret_error(ctx, 0);
    get_config_routine(ctx, &config);
}


static void destroy(WrapperContext *ctx)
{
    ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
    ifx_mimose_destroy(device);
    ret_error(ctx, 0);
}


static void start_acquisition(WrapperContext *ctx)
{
  ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);

  ifx_mimose_start_acquisition(device);

  ret_error(ctx, 0);
}

static void get_registers(WrapperContext* ctx)
{
    ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
    size_t register_list_size = ifx_mimose_get_register_count(device);
    uint32_t* registers = ifx_mem_alloc(sizeof(uint32_t) * register_list_size);
    if (registers == NULL)
    {
        ret_error(ctx, 0);
        ret_error(ctx, 1);
        goto fail;
    }
    ifx_mimose_get_registers(device, registers);
    if (ifx_error_get() != IFX_OK)
    {
        ret_error(ctx, 0);
        ret_error(ctx, 1);
        goto fail;
    }
    mxArray* plhs_1 = mxCreateNumericMatrix(register_list_size, 1, mxUINT32_CLASS, mxREAL);

    // pack register data into plhs_1 as one dimension array
    uint32_t* reg_ptr = mxGetData(plhs_1);

    for (size_t idx = 0; idx < register_list_size; ++idx)
    {
        *reg_ptr++ = registers[idx];
    }
    ret_error(ctx, 0);
    ret(ctx, 1, plhs_1);
fail:
    ifx_mem_free(registers);
}

static void set_register(WrapperContext* ctx)
{
    ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
    uint32_t registerval = arg_uint32(ctx, 1);

    ifx_mimose_set_registers(device, &registerval, 1);
    ret_error(ctx, 0);
}

static void get_register(WrapperContext* ctx)
{
    ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
    uint16_t register_address = arg_uint16(ctx, 1);
    uint16_t register_value = ifx_mimose_get_register_value(device, register_address);

    mxArray* plhs_0 = mxCreateNumericMatrix(1, 1, mxUINT16_CLASS, mxREAL); // for register value

    // pack result into plhs_0
    uint16_t* out0 = mxGetData(plhs_0);
    *out0 = register_value;
    ret_error(ctx, 0);
    ret(ctx, 1, plhs_0);
}

static void update_rc_lut(WrapperContext* ctx)
{
    ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
    ifx_mimose_update_rc_lut(device);
    ret_error(ctx, 0);
}


static void stop_acquisition(WrapperContext *ctx)
{
  ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);

  ifx_mimose_stop_acquisition(device);

  ret_error(ctx, 0);
}

static void get_next_frame(WrapperContext *ctx)
{
  ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
  ifx_mimose_start_acquisition(device);
  ifx_Cube_C_t* frame_ptr = ifx_mimose_get_next_frame(device, NULL, NULL);
  next_frame_routine(ctx, frame_ptr);
  ifx_cube_destroy_c(frame_ptr);
}


static void get_next_frame_timeout(WrapperContext *ctx)
{
  ifx_Mimose_Device_t* device = mimose_handle(ctx, 0);
  uint16_t timeout = arg_uint16(ctx, 1);
  ifx_mimose_start_acquisition(device);
  ifx_Cube_C_t* frame_ptr = ifx_mimose_get_next_frame_timeout(device, NULL, NULL, timeout);
  next_frame_routine(ctx, frame_ptr);
  ifx_cube_destroy_c(frame_ptr);
}


static const CommandDescriptor commands[] = {
    { "get_version", get_version, 2, 0 },
    { "get_version_full", get_version_full, 2, 0 },
    { "get_list", get_list, 2, 0 },
    { "create", create, 2, 0 },
    { "create_by_uuid", create_by_uuid, 2, 1 },
    { "set_config", set_config, 1, 9 },
    { "get_config", get_config, 9, 5 },
    { "get_config_defaults", get_config_defaults, 9, 5 },
    { "get_registers", get_registers, 2, 1},
    { "set_register", set_register, 1, 2},
    { "get_register", get_register, 2, 2},
    { "update_rc_lut", update_rc_lut, 1, 1 },
    { "destroy", destroy, 1, 1 },
    { "start_acquisition", start_acquisition, 1, 1 },
    { "stop_acquisition", stop_acquisition, 1, 1 },
    { "get_next_frame", get_next_frame, 5, 1 },
    { "get_next_frame_timeout", get_next_frame_timeout, 5, 2 },
    { NULL, NULL, 0, 0 }
};

const Wrapper wrapper = {
    "RadarDevice",
    commands
};
