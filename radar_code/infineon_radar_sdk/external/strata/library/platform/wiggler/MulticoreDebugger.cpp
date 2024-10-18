/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "MulticoreDebugger.hpp"

#include <common/Logger.hpp>
#include <platform/exception/EConnection.hpp>
#include <vector>


namespace
{
    const char dllName[]            = "mcdxdas.dll";
    const char serverName[]         = "UDAS";
    const uint32_t defaultFrequency = 30000000;
    const uint32_t rstClassVector   = 1;
}


MulticoreDebugger::MulticoreDebugger() :
    m_mcdInstance {dllName},
    m_core {NULL}
{
    // tx and txlist setup
    memset(&m_tx, 0, sizeof(m_tx));  // Set all to default values
    m_txlist.tx     = &m_tx;
    m_txlist.num_tx = 1;
    m_tx.num_bytes  = 4;
}

MulticoreDebugger::~MulticoreDebugger()
{
    MulticoreDebugger::close();
}

//void MulticoreDebugger::calibrate(uint8_t index)
//{
//    m_protocol.rifCalibrate(index);
//}

//void MulticoreDebugger::setFrameDimensions(uint16_t samples, uint16_t ramps, uint8_t inputChannels, uint8_t acquisitions)
//{

//}

//void MulticoreDebugger::setInputDataFormat(DataFormat_t format)
//{

//}

uint8_t MulticoreDebugger::getCore()
{
    return m_core->core_con_info->device_id;
}

bool MulticoreDebugger::isOpened()
{
    return (m_core != NULL);
}

void MulticoreDebugger::open(uint8_t i_system)
{
    if (isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Opening MulticoreDebugger...";

    mcd_return_et ret;
    //mcd_core_con_info_st  coreConInfo;
    uint32_t sv, tmp, numOpenServers, numSystems;

    // Load and initialize MCD API
    mcd_api_version_st versionReq;
    versionReq.v_api_major = MCD_API_VER_MAJOR;
    versionReq.v_api_minor = MCD_API_VER_MINOR;
    strcpy_s(versionReq.author, MCD_API_VER_AUTHOR);
    mcd_impl_version_info_st mcd_impl_info;
    if (!m_mcdInstance.lib_loaded())
    {
        throw EConnection("Could not load MCD library");
    }
    ret = m_mcdInstance.mcd_initialize_f(&versionReq, &mcd_impl_info);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not initialize MCD library");
    }

    //mcdt_print_mcd_impl_info(stdout, &mcd_impl_info);

    // Get IP address of server host
    const char host[] = "localhost";

    // System key
    const char system_key[MCD_KEY_LEN] = "";

    /*
    // Query number of running servers and start server if none is running
    numOpenServers = 0;
    ret = m_mcdInstance.mcd_qry_servers_f(host, TRUE, 0, &numOpenServers, 0);
    assert(ret == MCD_RET_ACT_NONE);
    */

    const uint32_t maxNumServers = 16;
    mcd_server_st *openServers[maxNumServers];

    numOpenServers = maxNumServers;

    char configString[128];
    //sprintf_s (configString, sizeof (configString), "McdHostName=\"%s\"\nMcdServerName=\"%s\"\nMcdAccHw.Frequency=%d ", host, serverName, defaultFrequency);
    // sprintf_s is broken in MinGW 5.3.0, use _snprinft_s instead
    _snprintf_s(configString, sizeof(configString), sizeof(configString), "McdHostName=\"%s\"\nMcdServerName=\"%s\"\nMcdAccHw.Frequency=%d ", host, serverName, defaultFrequency);

    // In case of Real HW, servers for all different Access HWs will be openend
    // If several boards are connected, all devices will be available for the selection process
    for (sv = 0; sv < numOpenServers; sv++)
    {

        ret = m_mcdInstance.mcd_open_server_f(system_key, configString, &openServers[sv]);

        if (ret != MCD_RET_ACT_NONE)
        {
            //assert (ret == MCD_RET_ACT_HANDLE_ERROR);
            mcd_error_info_st errInfo;
            m_mcdInstance.mcd_qry_error_info_f(0, &errInfo);
            break;  // while
        }
        //printf("%s\n", openServers[sv]->config_string);
    }

    numOpenServers = sv;
    if (numOpenServers == 0)
    {
        throw EConnection("Could not open any server");
    }

    m_server = openServers[0];

    //printf("\nSYSTEM LEVEL ##################################################################\n");

    // Number of systems
    numSystems = 0;
    ret        = m_mcdInstance.mcd_qry_systems_f(0, &numSystems, 0);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query MCD systems");
    }
    if (!numSystems)
    {
        throw EConnection("No MCD systems found");
    }

    /*
    printf("Found %d systems on host %s\n\n", numSystems, host);

    for (i = 0; i < numSystems; i++) {
      tmp = 1;
      ret = m_mcdInstance.mcd_qry_systems_f(i, &tmp, &coreConInfo);
      mcdt_print_core_con_info(stdout, &coreConInfo);
      printf("\n");
    }
    */

    // Select system
    mcd_core_con_info_st core_con_info_system_common;
    tmp = 1;
    ret = m_mcdInstance.mcd_qry_systems_f(i_system, &tmp, &core_con_info_system_common);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query selected MCD system");
    }


    //printf("\nDEVICE LEVEL ##################################################################\n");

    // Number of devices
    uint32_t num_devices = 0;
    ret                  = m_mcdInstance.mcd_qry_devices_f(&core_con_info_system_common, 0, &num_devices, 0);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query MCD devices");
    }
    if (!num_devices)
    {
        throw EConnection("No MCD devices found");
    }

    /*
    printf("Found %d devices within system %s\n\n", num_devices, core_con_info_system_common.system);

    for (i = 0; i < num_devices; i++) {
      tmp = 1;
      ret = m_mcdInstance.mcd_qry_devices_f(&core_con_info_system_common, i, &tmp, &coreConInfo);
      assert(ret == MCD_RET_ACT_NONE);
      mcdt_print_core_con_info(stdout, &coreConInfo);
      printf("\n");
    }
    */

    // Select device
    uint32_t i_device = 0;

    mcd_core_con_info_st core_con_info_device_common;
    tmp = 1;
    ret = m_mcdInstance.mcd_qry_devices_f(&core_con_info_system_common, i_device, &tmp,
                                          &core_con_info_device_common);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query selected MCD device");
    }


    //printf("\nCORE LEVEL ####################################################################\n");

    // Number of cores
    uint32_t num_cores = 0;  // Just to get the number of cores
    ret                = m_mcdInstance.mcd_qry_cores_f(&core_con_info_device_common, 0, &num_cores, 0);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query MCD cores");
    }
    if (!num_cores)
    {
        throw EConnection("No MCD cores found");
    }

    /*
    printf("Found %d cores within device %s\n\n", num_cores, core_con_info_device_common.device);

    for (i = 0; i < num_cores; i++) {
      tmp = 1;
      ret = m_mcdInstance.mcd_qry_cores_f(&core_con_info_device_common, i, &tmp, &coreConInfo);
      assert(ret == MCD_RET_ACT_NONE);
      assert(strcmp(coreConInfo.host, core_con_info_device_common.host) == 0);
      assert(strcmp(coreConInfo.system_key, core_con_info_device_common.system_key) == 0);
      assert(coreConInfo.device_key[0] == 0);  // Safe assumption for models
      assert(strcmp(coreConInfo.system, core_con_info_device_common.system) == 0);
      mcdt_print_core_con_info(stdout, &coreConInfo);
      printf("\n");
    }
    */

    // Select core
    uint32_t i_core = 0;

    mcd_core_con_info_st core_con_info_core;
    tmp = 1;
    ret = m_mcdInstance.mcd_qry_cores_f(&core_con_info_device_common, i_core, &tmp, &core_con_info_core);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query selected MCD core");
    }

    // Open core
    ret = m_mcdInstance.mcd_open_core_f(&core_con_info_core, &m_core);
    //mcdd_handle_err(stdout, 0, ret);
    if ((ret != MCD_RET_ACT_NONE) || (m_core == NULL))
    {
        throw EConnection("Could not open selected MCD core");
    }

    /*
    mcd_register_info_st core_ip_reg;
    mcdd_get_core_ip_addr(core, &core_ip_reg);

    // Close not needed open servers
    mcd_server_st *server = NULL; // Needed to set frequency of Access HW
    for (sv = 0; sv < numOpenServers; sv++) {
      if (mcdt_check_if_server_used(&core_con_info_core, openServers[sv]->config_string)) {
        assert(server == NULL);
        server = openServers[sv];
        continue;
      }
      ret = m_mcdInstance.mcd_close_server_f(openServers[sv]);
    }
    assert(server != NULL);
    */

    uint32_t rstClassVectorAvail;
    ret = m_mcdInstance.mcd_qry_rst_classes_f(m_core, &rstClassVectorAvail);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query MCD reset class vector");
    }
    if (!(rstClassVectorAvail & rstClassVector))
    {
        throw EConnection("Required reset class not available");
    }

    ret = m_mcdInstance.mcd_qry_max_payload_size_f(m_core, &m_maxPayload);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("Could not query MCD max. payload size");
    }
}

void MulticoreDebugger::close()
{
    if (!MulticoreDebugger::isOpened())
    {
        return;
    }

    LOG(DEBUG) << "Closing MulticoreDebugger";

    // Close core
    mcd_return_et ret;
    ret = m_mcdInstance.mcd_close_core_f(m_core);
    if (ret != MCD_RET_ACT_NONE)
    {
        LOG(ERROR) << "Could not close MCD core";
    }
    m_core = NULL;

    // Cleanup
    m_mcdInstance.mcd_exit_f();  // Enforce cleanup of all core and server connections
}

bool MulticoreDebugger::isRunning()
{
    mcd_return_et ret;
    mcd_core_state_st state;
    state.state = MCD_CORE_STATE_UNKNOWN;

    ret = m_mcdInstance.mcd_qry_state_f(m_core, &state);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("mcd_qry_state_f failed", static_cast<int>(ret));
    }

    return (state.state == MCD_CORE_STATE_RUNNING);
}

void MulticoreDebugger::reset(bool halt)
{
    mcd_return_et ret;

    ret = m_mcdInstance.mcd_rst_f(m_core, rstClassVector, halt ? TRUE : FALSE);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("mcd_rst_f failed", static_cast<int>(ret));
    }
}

void MulticoreDebugger::run()
{
    mcd_return_et ret;

    ret = m_mcdInstance.mcd_activate_trig_set_f(m_core);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("mcd_activate_trig_set_f failed", static_cast<int>(ret));
    }

    ret = m_mcdInstance.mcd_run_f(m_core, FALSE);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("mcd_run_f failed", static_cast<int>(ret));
    }
}

void MulticoreDebugger::stop()
{
    mcd_return_et ret;

    ret = m_mcdInstance.mcd_stop_f(m_core, FALSE);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("mcd_activate_trig_set_f failed", static_cast<int>(ret));
    }
}

void MulticoreDebugger::setFrequency(uint32_t frequency)
{
    mcd_return_et ret;
    ret = m_mcdInstance.set_acc_hw_frequency(m_server, &frequency);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("setFrequency failed", static_cast<int>(ret));
    }
}

uint32_t MulticoreDebugger::getMaxPayload() const
{
    return m_maxPayload;
}

uint32_t MulticoreDebugger::read(uint32_t address)
{
    mcd_return_et ret;

    uint32_t value    = 0xEEEEEEEE;
    m_tx.addr.address = address;
    ret               = m_mcdInstance.read32(m_core, &m_tx.addr, &value);  // Loader class utility function
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("readValue failed", static_cast<int>(ret));
    }
    return value;
}

void MulticoreDebugger::read(uint32_t address, uint32_t &value)
{
    value = read(address);
}

void MulticoreDebugger::write(uint32_t address, uint32_t value)
{
    //LOG(INFO) << "r = " << std::hex << address << "  - v = " << value;
    mcd_return_et ret;
    m_tx.addr.address = address;

    ret = m_mcdInstance.write32(m_core, &m_tx.addr, value);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("writeValue failed", static_cast<int>(ret));
    }
}

void MulticoreDebugger::write(uint32_t address, uint32_t count, const uint32_t data[])
{
    mcd_return_et ret;

    // Prepare transactions
    m_tx.addr.address = address;
    m_tx.access_type  = MCD_TX_AT_W;
    m_tx.data         = reinterpret_cast<uint8_t *>(const_cast<uint32_t *>(data));

    unsigned int numBytesRemaining = count * 4;
    while (numBytesRemaining > 0)
    {
        if (numBytesRemaining > m_maxPayload)
        {
            m_tx.num_bytes = m_maxPayload;
        }
        else
        {
            m_tx.num_bytes = numBytesRemaining;
        }

        ret = m_mcdInstance.mcd_execute_txlist_f(m_core, &m_txlist);

        numBytesRemaining -= m_tx.num_bytes_ok;

        if (ret != MCD_RET_ACT_NONE)
        {
            throw EConnection("write (mcd_execute_txlist_f) failed", static_cast<int>(ret));
        }

        m_tx.data += m_tx.num_bytes;
        m_tx.addr.address += m_tx.num_bytes;
    }
}

void MulticoreDebugger::writeBatch(const BatchType vals[], uint32_t count, bool /*optimize*/)
{
    mcd_return_et ret;

    mcd_txlist_st list;
    std::vector<mcd_tx_st> cmd;
    cmd.reserve(count);

    for (uint_fast32_t i = 0; i < count; i++)
    {
        // done at initialization: memset(&cmd[i], 0 , sizeof(cmd[i]));
        m_tx.num_bytes    = 4;
        m_tx.access_type  = MCD_TX_AT_W;
        m_tx.addr.address = vals[i].address;
        m_tx.data         = reinterpret_cast<uint8_t *>(const_cast<uint32_t *>(&vals[i].value));

        cmd.push_back(m_tx);
    }
    list.tx     = &cmd[0];
    list.num_tx = count;

    ret = m_mcdInstance.mcd_execute_txlist_f(m_core, &list);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("writeBatch failed", static_cast<int>(ret));
    }
    if (list.num_tx_ok != list.num_tx)
    {
        throw EConnection("writeBatch incomplete", static_cast<int>(list.num_tx_ok));
    }
}

void MulticoreDebugger::read(uint32_t address, uint32_t count, uint32_t data[])
{
    mcd_return_et ret;

    // Prepare transactions
    m_tx.addr.address = address;
    m_tx.access_type  = MCD_TX_AT_R;
    m_tx.data         = reinterpret_cast<uint8_t *>(data);

    unsigned int numBytesRemaining = count * 4;
    while (numBytesRemaining > 0)
    {
        if (numBytesRemaining > m_maxPayload)
        {
            m_tx.num_bytes = m_maxPayload;
        }
        else
        {
            m_tx.num_bytes = numBytesRemaining;
        }

        ret = m_mcdInstance.mcd_execute_txlist_f(m_core, &m_txlist);

        numBytesRemaining -= m_tx.num_bytes_ok;

        if (ret != MCD_RET_ACT_NONE)
        {
            throw EConnection("read (mcd_execute_txlist_f) failed", static_cast<int>(ret));
        }

        m_tx.data += m_tx.num_bytes;
        m_tx.addr.address += m_tx.num_bytes;
    }
}

void MulticoreDebugger::read16(uint32_t address, uint16_t &value)
{
    mcd_return_et ret;

    value             = 0xEEEE;
    m_tx.addr.address = address;
    ret               = m_mcdInstance.read16(m_core, &m_tx.addr, &value);  // Loader class utility function
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("readValue16 failed", static_cast<int>(ret));
    }
}

void MulticoreDebugger::write16(uint32_t address, uint16_t value)
{
    mcd_return_et ret;
    m_tx.addr.address = address;

    ret = m_mcdInstance.write16(m_core, &m_tx.addr, value);
    if (ret != MCD_RET_ACT_NONE)
    {
        throw EConnection("writeValue16 failed", static_cast<int>(ret));
    }
}

void MulticoreDebugger::setMem(uint32_t address, uint32_t value, uint32_t count)
{
    std::vector<uint32_t> m(count, value);
    write(address, count, m.data());
}

IMemory<uint32_t> *MulticoreDebugger::getIMemory()
{
    return this;
}
