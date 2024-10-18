/**
 * @copyright 2021 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <components/interfaces/IProtocolAvian.hpp>


struct
{
    uint32_t burstAddress;
    uint32_t burstSize;
} meta_data = {
    0xFFC00060,
    4096,
};


// This is the register dump of a BGT60TR13C read out with following configuration:
// Number of samples per chirp: 128
// Number of chirps per frame: 64
// RX antenna mask: 0x07
// TX antenna mask: 0x01
// Frame rate: 15 Hz
// TX power level: 31
// RF gain: 33 dB
// ADC sample rate: 1000000
// Lower frequency: 60500000000 Hz
// Upper frequency: 61500000000 Hz
// Chirp repetition time: 538.61e-6 s

const IProtocolAvian::Command default_doppler[] = {
    IProtocolAvian::Write(0x00, 0x1E827C),
    IProtocolAvian::Write(0x01, 0x140210),
    IProtocolAvian::Write(0x04, 0xE967FD),
    IProtocolAvian::Write(0x05, 0x0805B4),
    IProtocolAvian::Write(0x06, 0x102FFF),
    IProtocolAvian::Write(0x07, 0x010F00),
    IProtocolAvian::Write(0x08, 0x000000),
    IProtocolAvian::Write(0x09, 0x000000),
    IProtocolAvian::Write(0x0a, 0x000000),
    IProtocolAvian::Write(0x0b, 0x000BE0),
    IProtocolAvian::Write(0x0c, 0x000000),
    IProtocolAvian::Write(0x0d, 0x000000),
    IProtocolAvian::Write(0x0e, 0x000000),
    IProtocolAvian::Write(0x0f, 0x000000),
    IProtocolAvian::Write(0x10, 0x13FC51),
    IProtocolAvian::Write(0x11, 0x7FF41F),
    IProtocolAvian::Write(0x12, 0x703DEF),
    IProtocolAvian::Write(0x16, 0x000490),
    IProtocolAvian::Write(0x1d, 0x000480),
    IProtocolAvian::Write(0x24, 0x000480),
    IProtocolAvian::Write(0x2b, 0x000480),
    IProtocolAvian::Write(0x2c, 0x11BE0E),
    IProtocolAvian::Write(0x2d, 0x626C0A),
    IProtocolAvian::Write(0x2e, 0x03F000),
    IProtocolAvian::Write(0x2f, 0x787E1E),
    IProtocolAvian::Write(0x30, 0xE82498),
    IProtocolAvian::Write(0x31, 0x00009C),
    IProtocolAvian::Write(0x32, 0x000532),
    IProtocolAvian::Write(0x33, 0x000080),
    IProtocolAvian::Write(0x34, 0x000000),
    IProtocolAvian::Write(0x35, 0x000000),
    IProtocolAvian::Write(0x36, 0x000000),
    IProtocolAvian::Write(0x37, 0x26DB10),
    IProtocolAvian::Write(0x3f, 0x000100),
    IProtocolAvian::Write(0x47, 0x000100),
    IProtocolAvian::Write(0x4f, 0x000100),
    IProtocolAvian::Write(0x55, 0x000000),
    IProtocolAvian::Write(0x56, 0x000000),
    IProtocolAvian::Write(0x5b, 0x000000),

    IProtocolAvian::Write(0x00, 0x1E8271),
};
