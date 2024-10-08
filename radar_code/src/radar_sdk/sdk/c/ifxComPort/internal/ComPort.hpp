/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
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

#ifndef IFX_COMPORT_HPP
#define IFX_COMPORT_HPP

#include <cstdint>
#include <functional>
#include <list>
#include <string>
#include <utility>

struct com_s;  // Forward declaration of actual C Implementation

class ComPortInfo
{
    std::string m_name;
    std::string m_path;
    uint16_t m_vid;
    uint16_t m_pid;

public:
    ComPortInfo(std::string name,
                std::string path,
                uint16_t vid = uint16_t(0xFFFF),
                uint16_t pid = uint16_t(0xFFFF)) :
        m_name(std::move(name)),
        m_path(std::move(path)),
        m_vid(vid),
        m_pid(pid)
    {}

    std::string get_name() const
    {
        return m_name;
    }
    std::string get_path() const
    {
        return m_path;
    }
    uint16_t get_vid() const
    {
        return m_vid;
    }
    uint16_t get_pid() const
    {
        return m_pid;
    }

    static std::list<ComPortInfo> enumerate(std::function<bool(const ComPortInfo&)>& predicate);
    static uint32_t to_compat_buffer(const std::list<ComPortInfo>& ports, char* buffer, size_t buffer_size);
};

class ComPort
{
    com_s* m_com = nullptr;

public:
    ComPort() = delete;

    /** @brief Open port with baud rate
     *
     * Open the port port_name with baud rate baud_rate.
     *
     * Note that the constructor will not throw an exception if the port could
     * not be opened. Use the bool operator to check, for example:
     * @code
     *      ComPort port("COM17");
     *      if(!port)
     *          // opening port was not successful
     * @endcode
     *
     * @param [in]  port_name   name of port
     * @param [in]  baud_rate   baud rate
     */
    ComPort(const char* port_name, uint32_t baud_rate);
    ComPort(com_s* com) :
        m_com(com)
    {}
    ~ComPort();
    ComPort(const ComPort&) = delete;
    ComPort& operator=(const ComPort&) = delete;
    ComPort(ComPort&&) = delete;
    ComPort& operator=(ComPort&&) = delete;

    explicit operator bool() const
    {
        return m_com != nullptr;
    }

    void set_timeout(uint32_t timeout_period_ms);
    size_t send_data(const void* data, size_t num_bytes);
    size_t get_data(void* data, size_t num_requested_bytes);
    char get_char();
    void close();
    com_s* disown();
};


#endif
