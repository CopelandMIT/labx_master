/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "RFS SDK Documentation", "index.html", [
    [ "About this Document", "index.html", [
      [ "Scope and Purpose", "index.html#about-scope-purpose", null ],
      [ "Installation", "index.html#about-installation", null ],
      [ "Intended Audience", "index.html#about-intended-audience", null ],
      [ "About this documentation", "index.html#about", null ]
    ] ],
    [ "Introduction to Radar", "pg_radarsdk_introduction.html", [
      [ "Radar System", "pg_radarsdk_introduction.html#sct_radarsdk_introduction_radarsystem", null ],
      [ "Classification of Radar Systems", "pg_radarsdk_introduction.html#sct_radarsdk_introduction_radarsystemtypes", null ],
      [ "Range Resolution", "pg_radarsdk_introduction.html#sct_radarsdk_introduction_rangeres", null ],
      [ "Range Doppler", "pg_radarsdk_introduction.html#sct_radarsdk_introduction_rangedopp", null ],
      [ "Radar parameters explained", "pg_radarsdk_introduction.html#sct_radarsdk_introduction_parametersexplained", null ],
      [ "Radar parameters as metrics", "pg_radarsdk_introduction.html#sct_radarsdk_introduction_metrics", null ]
    ] ],
    [ "Introduction to Radar SDK", "pg_radarsdk_introduction_radar_sdk.html", [
      [ "Overview", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_overview", null ],
      [ "The C library", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_c", [
        [ "Overview and Architecture", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_architecture", null ],
        [ "Conventions", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_update_conventions", null ],
        [ "Object-Oriented Programming and Memory Management", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_oop", null ],
        [ "Error Handling", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_error_handling", null ],
        [ "Thread safety", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_threads", null ],
        [ "Logging", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_logging", null ],
        [ "3rd Party Libraries used by Radar SDK", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_3rdparty", null ]
      ] ],
      [ "Typical Problems and Pitfalls", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_common_problems", [
        [ "Device Not Found", "pg_radarsdk_introduction_radar_sdk.html#ssct_radarsdk_nodevice", null ]
      ] ],
      [ "Device Access", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_device_access", [
        [ "libAvian", "pg_radarsdk_introduction_radar_sdk.html#ssct_radarsdk_libavian", null ],
        [ "Strata/Stratula", "pg_radarsdk_introduction_radar_sdk.html#ssct_radarsdk_strata", null ],
        [ "DeviceControl", "pg_radarsdk_introduction_radar_sdk.html#ssct_radarsdk_devicecontrol", null ],
        [ "Interaction between DeviceControl, Strata/Stratula, and libAvian", "pg_radarsdk_introduction_radar_sdk.html#ssct_radarsdk_interaction", null ],
        [ "Combining DeviceControl and libAvian", "pg_radarsdk_introduction_radar_sdk.html#ssct_radarsdk_devicecontrol_libavian", null ]
      ] ],
      [ "Moving to an embedded platform", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_driver", [
        [ "Exporting a Device Configuration to a Firmware project", "pg_radarsdk_introduction_radar_sdk.html#sct_radarsdk_introduction_radar_sdk_registers", null ]
      ] ]
    ] ],
    [ "Building SDK from source code", "pg_radarsdk_setup_build_environment.html", [
      [ "Windows", "pg_radarsdk_setup_build_environment.html#ssct_radarsdk_setup_build_environmenton_windows", [
        [ "MinGW", "pg_radarsdk_setup_build_environment.html#windows_mingw", null ],
        [ "Visual Studio", "pg_radarsdk_setup_build_environment.html#windows_vs", null ],
        [ "WSL (Windows Subsystem for Linux)", "pg_radarsdk_setup_build_environment.html#windows_wsl", null ]
      ] ],
      [ "Linux", "pg_radarsdk_setup_build_environment.html#ssct_radarsdk_setup_build_environmenton_linux", [
        [ "Ubuntu 22.04 / Raspbian Buster", "pg_radarsdk_setup_build_environment.html#linux_build", null ]
      ] ],
      [ "Python Wheels", "pg_radarsdk_setup_build_environment.html#ssct_radarsdk_setup_build_environmenton_wheels", null ]
    ] ],
    [ "Device configuration guidelines", "pg_radarsdk_device_config_guide.html", [
      [ "Radar device configuration parameters", "pg_radarsdk_device_config_guide.html#ssct_radarsdk_device_config_guide_params", null ],
      [ "Measurement Frame", "pg_radarsdk_device_config_guide.html#sct_radarsdk_device_config_guide_measurement_frame", null ],
      [ "Virtual Rx antenna ordering", "pg_radarsdk_device_config_guide.html#sct_radarsdk_device_config_guide_virtual_antenna_order", null ],
      [ "Code examples", "pg_radarsdk_device_config_guide.html#sct_radarsdk_device_config_guide_code_examples", null ]
    ] ],
    [ "JSON Configuration", "pg_radarsdk_json.html", [
      [ "Configuration structure", "pg_radarsdk_json.html#json_config_structure", [
        [ "Schema of device", "pg_radarsdk_json.html#json_schema_device", null ]
      ] ],
      [ "Schema of device_info", "pg_radarsdk_json.html#json_device_info", null ],
      [ "Schema of device_config", "pg_radarsdk_json.html#json_device_config", [
        [ "Schema of fmcw_scene", "pg_radarsdk_json.html#json_schema_fmcw_scene", null ],
        [ "Schema of fmcw_single_shape", "pg_radarsdk_json.html#json_schema_fmcw_single_shape", null ],
        [ "Schema of fmcw_device_config", "pg_radarsdk_json.html#json_schema_fmcw_device_config", null ],
        [ "Schema of doppler_ltr11", "pg_radarsdk_json.html#doppler_ltr11", null ]
      ] ],
      [ "Segmentation configuration", "pg_radarsdk_json.html#json_schema_segmentation", null ],
      [ "Presence Sensing", "pg_radarsdk_json.html#json_schema_presence_sensing", null ],
      [ "Advanced Motion Sensing Algorithm", "pg_radarsdk_json.html#json_schema_advanced_motion_sensing", null ],
      [ "Example configurations", "pg_radarsdk_json.html#json_schema_examples", [
        [ "Using fmcw_scene", "pg_radarsdk_json.html#json_schema_examples_scene", null ],
        [ "Using fmcw_single_shape", "pg_radarsdk_json.html#json_schema_examples_single_shape", null ],
        [ "Segmentation configuration", "pg_radarsdk_json.html#json_schema_examples_with_segmentation", null ],
        [ "Configuration for multiple devices", "pg_radarsdk_json.html#json_schema_examples_multiple_devices", null ]
      ] ]
    ] ],
    [ "Algorithms supported by Radar SDK", "pg_radarsdk_algorithms.html", [
      [ "Moving Target Indicator", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_mti", null ],
      [ "2D Moving Target Indicator", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_2dmti", null ],
      [ "Range-Doppler Map", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_rdm", null ],
      [ "Range-Angle Image", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_rai", null ],
      [ "Digital Beam Former (DBF)", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_dbf", null ],
      [ "Ordered Statistics Constant False Alarm Rate (OS-CFAR)", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_detect_oscfar", null ],
      [ "Density-Based Spatial Clustering of Applications with Noise (DBSCAN)", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_detect_dbscan", null ],
      [ "Capon", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_detect_capon", null ],
      [ "Segmentation", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_segmentation", null ],
      [ "Presence Sensing", "pg_radarsdk_algorithms.html#ssct_radarsdk_algorithms_presencesensing", null ]
    ] ],
    [ "Example applications", "pg_radarsdk_applications.html", [
      [ "Presence Sensing APP", "pg_radarsdk_applications.html#ssct_radarsdk_applications_presence", [
        [ "Installation procedure", "pg_radarsdk_applications.html#sssct_radarsdk_applications_presence_detection_ip", null ],
        [ "Operation of FMCW based system", "pg_radarsdk_applications.html#ssct_app_presence_fmcw", null ],
        [ "Radar device configuration", "pg_radarsdk_applications.html#ssct_app_presence_devcnfg_param", null ],
        [ "Presence sensing algorithm configuration", "pg_radarsdk_applications.html#ssct_app_presence_psalgocnfg_param", null ]
      ] ],
      [ "Segmentation APP", "pg_radarsdk_applications.html#ssct_radarsdk_applications_segmentation", [
        [ "Installation procedure", "pg_radarsdk_applications.html#sssct_radarsdk_applications_segmentation_ip", null ],
        [ "Radar device configuration", "pg_radarsdk_applications.html#ssct_app_segmentation_devcnfg_param", null ],
        [ "Segmentation antenna configuration", "pg_radarsdk_applications.html#ssct_app_segmentation_psantencnfg_param", null ],
        [ "Segmentation algorithm configuration", "pg_radarsdk_applications.html#ssct_app_segmentation_psalgocnfg_param", null ]
      ] ],
      [ "Raw Data APP", "pg_radarsdk_applications.html#ssct_radarsdk_applications_raw_data", [
        [ "Installation procedure", "pg_radarsdk_applications.html#sssct_radarsdk_applications_raw_data_ip", null ]
      ] ],
      [ "Recorder APP", "pg_radarsdk_applications.html#ssct_radarsdk_applications_recorder", [
        [ "Installation procedure", "pg_radarsdk_applications.html#sssct_radarsdk_applications_recorder_ip", null ]
      ] ],
      [ "Continuous Wave APP", "pg_radarsdk_applications.html#ssct_radarsdk_applications_cw", [
        [ "Installation procedure", "pg_radarsdk_applications.html#sssct_radarsdk_applications_cw_ip", null ]
      ] ]
    ] ],
    [ "References", "pg_radarsdk__ref.html", [
      [ "List of references", "pg_radarsdk__ref.html#ref_list", null ],
      [ "List of terms and abbreviations", "pg_radarsdk__ref.html#terms_list", null ]
    ] ],
    [ "Python Wrapper", "pg_rdk_python.html", [
      [ "Getting Started", "pg_rdk_python.html#sct_rdk_python_quick", [
        [ "Installation", "pg_rdk_python.html#sct_rdk_python_installation", null ],
        [ "Help and Documentation", "pg_rdk_python.html#sct_rdk_python_help_function", null ]
      ] ],
      [ "FMCW 60GHz Radars", "pg_rdk_python.html#sct_rdk_python_fmcw_60ghz", [
        [ "Running the examples", "pg_rdk_python.html#sct_rdk_python_running_the_example", null ],
        [ "Running the example applications", "pg_rdk_python.html#sct_rdk_python_running_applications", null ],
        [ "Writing a Radar application using Python", "pg_rdk_python.html#sct_rdk_python_algo", [
          [ "Importing the Python wrapper", "pg_rdk_python.html#sct_rdk_python_algo_import", null ],
          [ "Connecting to the Radar sensor", "pg_rdk_python.html#sct_rdk_python_algo_connect", null ],
          [ "Setting a Radar configuration", "pg_rdk_python.html#sct_rdk_python_algo_config", null ],
          [ "Fetching Radar data", "pg_rdk_python.html#sct_rdk_python_algo_data", null ]
        ] ],
        [ "A presence sensing application", "pg_rdk_python.html#sct_rdk_python_running_the_algoexample", [
          [ "Configuring the Radar and algorithm", "pg_rdk_python.html#sct_rdk_python_presence_config", null ],
          [ "Computing Radar distance data", "pg_rdk_python.html#sct_rdk_python_fft_spectrum", null ],
          [ "Presence sensing algorithm", "pg_rdk_python.html#sct_rdk_python_presence_algo", null ],
          [ "Extension into an anti-peeking application", "pg_rdk_python.html#sct_rdk_python_algoext", null ]
        ] ]
      ] ]
    ] ],
    [ "MATLAB Wrapper", "pg_rdk_matlab.html", [
      [ "1    Quick start guide", "pg_rdk_matlab.html#sct_rdk_matlab_quick", [
        [ "1.1 Requirements", "pg_rdk_matlab.html#sct_rdk_matlab_requirements", null ],
        [ "1.2  Location of wrapper in release package", "pg_rdk_matlab.html#sct_rdk_matlab_extracting_the_package", null ],
        [ "1.3   Start MATLAB", "pg_rdk_matlab.html#sct_rdk_matlab_start", null ],
        [ "1.4 Run the example", "pg_rdk_matlab.html#sct_rdk_matlab_run_the_example", null ],
        [ "1.5   Help function", "pg_rdk_matlab.html#sct_rdk_matlab_help_function", null ]
      ] ],
      [ "2  Writing a Radar application using MATLAB", "pg_rdk_matlab.html#sct_rdk_matlab_algo", [
        [ "2.1 Accessing the MATLAB wrapper", "pg_rdk_matlab.html#sct_rdk_matlab_algo_import", null ],
        [ "2.2 Connecting to the Radar sensor", "pg_rdk_matlab.html#sct_rdk_matlab_algo_connect", null ],
        [ "2.3 Setting a Radar configuration", "pg_rdk_matlab.html#sct_rdk_matlab_algo_config", null ],
        [ "2.4 Fetching Radar data", "pg_rdk_matlab.html#sct_rdk_matlab_algo_data", null ],
        [ "2.5 A presence sensing application", "pg_rdk_matlab.html#sct_rdk_matlab_running_the_algoexample", [
          [ "2.5.1 Configuring the Radar and algorithm", "pg_rdk_matlab.html#sct_rdk_matlab_presence_config", null ],
          [ "2.5.2 Computing Radar distance data", "pg_rdk_matlab.html#sct_rdk_matlab_fft_spectrum", null ],
          [ "2.5.3 Presence sensing algorithm", "pg_rdk_matlab.html#sct_rdk_matlab_presence_algo", null ]
        ] ],
        [ "2.6 Extension into anti-peeking application", "pg_rdk_matlab.html#sct_rdk_matlab_algoext", null ]
      ] ]
    ] ],
    [ "Modules", "modules.html", "modules" ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Variables", "functions_vars.html", "functions_vars" ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"2_d_m_t_i_8h.html",
"_error_8h.html#a579b35c305c27de6f162e47dc7c43851a862a7ace998823a79b86178a2e4c10b1",
"functions_h.html",
"group__gr__cube.html#ga1cba778c0d00b6ed690c8ccf3cb7b20d",
"group__gr__devicefmcw.html#ga891b8f2e39994cbaa00a4d90b5f0dac6",
"group__gr__matrix.html#gad4b805bbd8da8cf95e2d66b0fc3a7737",
"group__gr__rdm.html#ga9d73602bb619c8a90a93310467771f59",
"group__gr__utilities.html#gaffd20b7a3128b02a1dfc9e0199b716db",
"pg_rdk_matlab.html#sct_rdk_matlab_algo_config",
"structifx___radar___sensor___info__t.html#a0a43055d9eec2c0cead6b7b6bec8ceba"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';