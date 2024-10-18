/**
 * \file ifxAvian_IPort.hpp
 *
 * This file declares an interface class for a port that is connected to an
 * Avian device.
 */
/* ===========================================================================
** Copyright (C) 2020-2023 Infineon Technologies AG
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


#ifndef IFX_AVIAN_IPORT_H
#define IFX_AVIAN_IPORT_H

// ---------------------------------------------------------------------------- includes
#include <cstddef>
#include <cstdint>
#include <functional>

// ---------------------------------------------------------------------------- namespaces
namespace Infineon {
namespace Avian {
namespace HW {

// ---------------------------------------------------------------------------- Spi_Command_t
/**
 * \brief This type represents a command word that is sent to an Avian device
 *        during a (non-burst) SPI transfer.
 *
 * In a non-burst SPI transfer a 32 bit command word is sent to the Avian device.
 * A command word contains the following bit fields:
 * - Bit 31 - 25: Register address (7 Bit)
 * - Bit      24: 1 for command "write", 0 for command "read"
 * - Bit 23 -  0: Register value for write command (24 Bit)
 */
typedef uint32_t Spi_Command_t;

// ---------------------------------------------------------------------------- Spi_Response_t
/**
 * \brief This type represents a word that is received from an Avian device
 *        during a (non-burst) SPI transfer.
 *
 * In a non-burst SPI transfer the Avian device sends a 32 bit word to the SPI
 * master. The received word contain the following bit fields:
 * - Bit 31 - 24: Status Register GSR0 (8 Bit)
 * - Bit 23 -  0: Register value (24 Bit)
 */
typedef uint32_t Spi_Response_t;

// ---------------------------------------------------------------------------- Packed_Raw_Data_t
/**
 * \brief This type represent the native data format produced by Avian devices.
 *
 * Avian devices acquire radar data as 12bit ADC values. When acquired data is
 * read from an Avian device through the SPI connection, those ADC values are
 * transmitted without any padding bits, so \ref IReadPort instances that use
 * this raw data format return arrays of packed data to the application where
 * three bytes hold two ADC values.
 *
 * Packed ADC values can be unpacked similar to the following example:
 * \code
 * Packed_Raw_Data_t packed_pair[3];
 *
 * // ...
 * // Assumption is that packed_pair now holds 2 values.
 *
 * uint16_t first_value = (uint16_t(packed_pair[0]) << 4)
 *                        | (packed_pair[1] >> 4);
 * uint16_t second_value = (uint16_t(packed_pair[1] & 0x0F) << 8)
 *                         | packed_pair[2];
 * \endcode
 */
typedef uint8_t Packed_Raw_Data_t;

// ---------------------------------------------------------------------------- IControlPort
/**
 * \brief This class is an interface for configuration of an Avian device.
 *
 * This interface class provides functionality that is required to
 * configure an Avian device. Therefore it provides methods to reset an
 * Avian device and to read and write configuration registers of the devices.
 * Since the configuration of an Avian device usually must take some
 * properties of the hardware connection into account, this interface also
 * provides a method to read those properties.
 *
 * For certain testing use cases, this interface also provides a method to
 * read the level IRQ pin. This is usually not needed for normal operation.
 *
 * Usage Example
 * -------------
 * The following code example shows a simple application skeleton that
 * demonstrates how in IControl port is used to setup an Avian device.
 *
 * \code
 * using namespace Infineon::Avian::HW;
 *
 * // Initialization of the port
 * // --------------------------
 * // Here it is assumed that ActualPortImplementation is a class derived
 * // from IPort and fully implements the interface of IPort. In a real world
 * // example the constructor may look differently. In an embedded
 * // implementation the interface might not be constructed dynamically.
 * // For this example also an IControlPort pointer could have been used rather
 * // than an IPort pointer, but this would not be how real world code would
 * // look like.
 * IPort<Packed_Raw_Data_t>* port = new ActualPortImplementation();
 *
 * // Initialization of the Avian Device
 * // ----------------------------------
 * const size_t num_config_words = 42;
 * Spi_Command_t command_words[num_config_words];
 *
 * // Here the application must setup the command words to configure the Avian
 * // device. It may call the method get_properties to find out how to program
 * // the FIFO control registers. Please note that 42 is just an arbitrary
 * // value. The number of configuration words depends on the use case!
 * // ...
 *
 * port->generate_reset_sequence();
 * port->send_commands(command_words, num_config_words);
 *
 * // Data Acquisition
 * // ----------------
 * // The application must initialize the trigger word according to its needs...
 * Spi_Command_t trigger_word;
 * port->send_commands(&trigger_word, 1);
 *
 * // Reading acquired data is beyond the scope of this example, so
 * // process_loop() is used as a place holder. It is assumed that process_loop
 * // expects an IReader instance to read data from as an argument. That's the
 * // reason why this example uses an IPort pointer instead of an IControlPort
 * // pointer.
 * process_loop(port);
 *
 * // Clean Up
 * // --------
 * delete port;
 * \endcode
 *
 * \note Implementors usually should not derive from this class directly.
 *       Instead the base class \ref IPort should be used, because it also
 *       contains the interface to read data from an Avian device.
 *       Parts of an applications that are not dealing with processing of
 *       radar data may use a pointers or references of this base class type.
 */
class IControlPort
{
public:
    /**
     * \brief This structure contains information about the connection that may
     *        be relevant for an application.
     */
    struct Properties
    {
        /**
         * \brief A zero terminated string describing the port.
         */
        const char* description;

        /**
         * \brief This flag indicates if the Avian device must be configured
         *        for SPI high speed compensation mode.
         *
         * By default the Avian device transmits data on the falling clock edge and
         * SPI master reads the data on the rising clock edge. Some SPI devices may
         * not be able to achieve high SPI clock rates around 50MHz.
         *
         * High speed SPI reads can be achieved by modifying the SPI timing in two
         * ways:
         * 1. The save can prepone transmission of data by a half clock cycle. In
         *    this case data is written to the SPI lines on the rising clock edge,
         *    and the SPI master reads the data on the following rising clock edge.
         * 2. The SPI master postpones data reading by a half clock cycle. In this
         *    case the Avian device transmits data to the SPI lines on the falling
         *    clock edge as usual and the SPI master reads that data with the
         *    following falling clock edge.
         *
         * Either of those two modifications is suitable, but they can't be
         * combined. To use modification 1 the SPI master must be configured for
         * standard SPI timing and the high_speed_compensation field must be set
         * to true. To use modification 2 the SPI master must be configured
         * accordingly and the high_speed_compensation must be set to false.
         */
        bool high_speed_compensation;

        /**
         * \brief The number of clock cycles generated by the SPI master
         *        between the address phase and the data phase.
         *
         * A quad SPI burst read out consists of three phases. After CS goes low,
         * the master sends an 8 bit read address to the slave. The address phase
         * is followed by an idle phase. During the idle phase, the SPI CLK signal
         * keeps on clocking, but no data is transferred. During this idle phase
         * both SPI master and slave prepare for the data read out. Finally the
         * data is transmitted in the third phase.
         *
         * The Avian device can be configured for idle phases of up to 16 clock
         * cycles, and requires at least two wait cycles. The requirements of the
         * SPI master may be different. The field quad_spi_wait_cycles contains
         * the number of wait cycles the SPI master is configured for.
         */
        uint8_t quad_spi_wait_cycles;
    };

    /**
     * \brief The destructor closes the port.
     *
     * In this interface class, the destructor does nothing, but it must be
     * declared virtual, because an application might access a derived
     * instance through a pointer to the interface class.
     */
    virtual ~IControlPort() = default;

    /**
     * \brief This method returns the properties of the IControlPort instance.
     */
    virtual const Properties& get_properties() const = 0;

    /**
     * \brief This method sends SPI commands to the Avian device.
     *
     * This method sends the provided array of command words via
     * SPI to the Avian device connected to the port and receives the
     * response words sent back from the Avian device simultaneously. The
     * command words are transfered in the order, they are stored in the
     * array.
     *
     * The argument response may be nullptr. In this case received
     * response words are ignored and not returned to the caller.
     *
     * This method always uses standard SPI (CS, CLK, MISO and MOSI). Quad
     * SPI is not used.
     *
     * For SPI communication chip select (CS) is LOW active and is controlled
     * by this method. Configuration words are transferred in frames of
     * 32bit. CS goes low before each frame and high again after every frame.
     * CS must not stay low between transmission of two frames. Data words are
     * sent with MSB first.
     *
     * Inactive CLK level is LOW. Data on MOSI pin changes on falling CLK edge,
     * and is read in by the Avian device on rising edge. This is also known as
     * SPI mode 0 (CPOL=0, CPHA=0).
     *
     * Basically an Avian device sends data to MISO also on falling clock edge
     * and master should read data on rising clock edge. Anyway, for high SPI
     * clock rates this timing does not work and a timing modification is
     * needed. See \ref Properties for more information about high speed
     * timing.
     *
     * This method disables interrupt handling while the SPI transfer is in
     * progress.
     *
     * \param[in]  commands   The array of command words to be sent.
     * \param[in]  num_words  The number of configuration words to be
     *                        transferred.
     * \param[out] response   The array where received words are stored.
     */
    virtual void send_commands(const Spi_Command_t* commands, size_t num_words,
                               Spi_Response_t* response = nullptr) = 0;

    /**
     * \brief This method generates a hardware reset sequence at the input
     *        pins of the connected Avian device.
     *
     * Because the reset input of the Avian device is multiplexed with the quad
     * SPI data line DIO3 the SPI CS signal must be HIGH all time during a reset
     * condition.
     *
     * The reset signal of the connected Avian device must be driven low
     * and kept low for at least 1000ns, before going HIGH again.
     */
    virtual void generate_reset_sequence() = 0;

    /**
     * \brief This method reads the logic level of the IRQ line.
     *
     * This method allows to check, if the level of the IRQ pin of the
     * connected Avian device is LOW or HIGH.
     *
     * \return This method returns false if the logic level of the IRQ line is
     *         LOW and true if the logic level is HIGH.
     */
    virtual bool read_irq_level() = 0;
};

// ---------------------------------------------------------------------------- IReadPort
/**
 * \brief This class is an interface to read data from an Avian device.
 *
 * An IReaderPort instance takes care for reading acquired radar data from an
 * Avian device in the background. When the Avian device rises the level of the
 * IRQ pin, the IReaderPort instance reads radar data in an SPI burst transfer.
 * The read data is stored in memory buffers provided by the application. If no
 * memory buffer is provided, no radar data read is performed. Before data can
 * be read the application must use the method \ref start_reader to configure
 * the command word to initiate the burst transfer and the amount of data words
 * to be read in each burst transfer. That method also expects an
 * \ref Data_Ready_Callback_t function, which is called at the end of each
 * burst transfer. Once the data reading has been configured the application
 * must use the method \ref set_buffer to pass a memory buffer to the
 * IReadPort instance. The provided buffer must be big enough to hold the data
 * of a single burst transfer as it was configured before. As soon as the
 * memory buffer is filled, the IReaderPort instance invokes the previously
 * registered callback function. For continuous data reading the callback
 * method should use \ref set_buffer again to provide the next memory
 * buffer.
 *
 * \tparam DATA_TYPE  This is the data type used for the acquired data values.
 *
 * \note In an bare metal firmware context, the callback function may be
 *       executed in interrupt context, and so no exhaustive computing must be
 *       done in the callback routine. The callback routine has two main jobs:
 *       1) Provide a new memory buffer to the IReadPort to keep the reading
 *          process running.
 *       2) Notify another thread (main loop or a real processing thread) that
 *          data is ready for processing.
 *
 * Usage Example
 * -------------
 * This is a simple example that shows how acquired data can be read by an
 * application through the IReadPort interface.
 *
 * \code
 * using namespace Infineon::Avian::HW;
 *
 * void process_loop(IReaderPort<uint16_t>* port)
 * {
 *   // Provide the data buffer for readout
 *   const unsigned block_size = 4096; // this is just an example value!
 *   uint16_t* data_buffer1 = new uint16_t[block_size];
 *   uint16_t* data_buffer2 = new uint16_t[block_size];
 *   bool data_ready_flag = false;
 *   port->start_reader(0xFFFFFFFF, block_size,
 *                      [&data_ready_flag](Spi_Response_t)-> void
 *                      { data_ready_flag = true; });
 *   port->set_buffer(data_buffer1);
 *
 *   while (1) // insert some more sensible condition here!
 *   {
 *     // check if data is available
 *     if (data_ready_flag > 0)
 *     {
 *       // first provide a new buffer to keep the data fetching in the
 *       // background working.
 *       data_ready_flag = false;
 *       port->set_buffer(data_buffer2);
 *
 *       // This function is of course just a place holder.
 *       process_data(data_buffer1);
 *
 *       // swap the buffers for the next iteration
 *       uint16_t* temp = data_buffer2;
 *       data_buffer2 = data_buffer1;
 *       data_buffer1 = temp;
 *     }
 *     else
 *     {
 *       // do something else ...
 *     }
 *   }
 *   delete[] data_buffer1;
 *   delete[] data_buffer2;
 * }
 * \endcode
 */
template <typename DATA_TYPE>
class IReadPort
{
public:
    /**
     * \brief The destructor closes the port.
     *
     * In this interface class, the destructor does nothing, but it must be
     * declared virtual, because an application might access a derived
     * instance through a pointer to the interface class.
     */
    virtual ~IReadPort() = default;

    /**
     * \brief This type defines a callback function for notification when new
     *        data has been received from the Avian device.
     *
     * A function of this type must be passed to \ref start_reader.
     * That function is then called at the end of each data readout burst
     * transfer.
     *
     * The argument of the callback function is the response word that was
     * received from the Avian device while the burst command was sent.
     */
    typedef std::function<void(Spi_Response_t)> Data_Ready_Callback_t;

    /**
     * \brief This method initializes the SPI burst parameters.
     *
     * In this method all data reading related parameters are set that usually don't
     * change during operation.
     *
     * This method must be called before any IF data can be read from the Avian
     * device. It configures the IRQ handler that is invoked when the IRQ pin
     * of the Avian device goes to high level. The IRQ handler starts an SPI
     * burst transfer to read IF data into the buffer provided by the
     * application through \ref set_buffer. The burst transfer is always
     * initiated with the provided command words and reads always the specified
     * number of 12 bit data words. At the end of a burst transfer the provided
     * callback function is called.
     *
     * See also \ref Infineon::Avian::HW::IReadPort.
     *
     * \param[in] burst_command  The command word that is sent to the
     *                           Avian device to initiate a burst transfer.
     * \param[in] burst_size     The number of 12 bit data words that are
     *                           read from the Avian device in each burst
     *                           transfer.
     * \param[in] callback       A callback function that is invoked at the end
     *                           of each burst transfer.
     */
    virtual void start_reader(Spi_Command_t burst_command, size_t burst_size,
                              Data_Ready_Callback_t callback) = 0;

    /**
     * \brief This method stops any ongoing data reading
     *
     * All parameters set during \ref start_reader are cleared and the
     * currently used buffer is dropped.
     * When this method returns no more data ready callbacks are invoked.
     */
    virtual void stop_reader() = 0;

    /**
     * \brief This method passes a memory buffer for data readout to the
     *        IReadPort instance.
     *
     * This method provides a memory block to the IPort instance, that is used
     * to store IF data that is received from the front end device. The buffer
     * must be big enough to hold that number of data words, that was specified
     * before to the method \ref start_reader.
     *
     * This method must be called before any data read transfer can happen. It
     * should also be called from the \ref Data_Ready_Callback_t function to
     * provide another buffer for next transfer.
     *
     * See also \ref Infineon::Avian::HW::IReadPort.
     *
     * \param[in] buffer  A memory buffer where the data received during the
     *                    next burst transfer is stored.
     */
    virtual void set_buffer(DATA_TYPE* buffer) = 0;
};

// ---------------------------------------------------------------------------- IPort
/**
 * \brief This class is an interface for a port an Avian device is connected to.
 *
 * This class provides a hardware abstraction layer to access a radar front
 * end device of the Avian family (BGT60TR13C, BGT60ATR24C, BGT60TR13D,
 * BGT60TR12E, BGT60TR11D).
 * The interface is split into two parts. \ref IControlPort provides
 * functionality to configure the Avian device, \ref IReadPort provides
 * functionality to read acquired IF data from the Avian radar front end.
 *
 * The implementation of the IPort interface is very platform specific,
 * so each platform must provide its own implementation as a derived class. It
 * is strongly recommended to derive an implementation from IPort rather than
 * directly from \ref IControlPort and \ref IReadPort, because a single IPort
 * pointer is easier to handle for an application.
 *
 * An application must create a separate instance of IPort for each Avian
 * device it wants to use and delete that instance if the port is no longer
 * needed. Parts of an application may refer to the port through pointers or
 * references of \ref IControlPort or \ref IReadPort if they deal either with
 * device setup or with data processing only.
 *
 * \tparam DATA_TYPE  This is the data type returned through the \ref IReadPort
 *                   interface.
 *
 * \note It is recommended to use \ref Packed_Raw_Data_t as DATA_TYPE for
 *       platform specific implementations of IPort, and simply return the
 *       received data as is to the application through the \ref IReadPort
 *       interface. Application can use a \ref DataConverter to turn packed
 *       raw data into the desired format.
 *       Anyhow, in an embedded context using a different data format may be
 *       more efficient. Some SPI controllers allow unpacking and padding in
 *       hardware (e.g. store 12bit values as 16bit integers). If an embedded
 *       application uses a data format that can be delivered by the platform's
 *       SPI controller, that data format may be the better choice for that
 *       very use case.
 */
template <typename DATA_TYPE>
class IPort : public IControlPort, public IReadPort<DATA_TYPE>
{};

/* ------------------------------------------------------------------------ */
}  // namespace HW
}  // namespace Avian
}  // namespace Infineon

#endif /* IFX_AVIAN_IPORT_H */

/* --- End of File -------------------------------------------------------- */
