/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */


#include <platform/serial/BoardSerial.hpp>  // this needs to be included before the Enumerator for correct forward declarations

#include "EnumeratorSerialImpl.hpp"

#include <iterator>


namespace
{
    const char *devList[] = {
        "/dev/ttyAMA*",
        "/dev/ttyACM*",
        "/dev/ttyUSB*",
    };
}


EnumeratorSerialImpl::EnumeratorSerialImpl() :
    EnumeratorSerialImplBase(std::begin(devList), std::end(devList))
{
}
