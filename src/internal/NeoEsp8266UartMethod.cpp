/*-------------------------------------------------------------------------
NeoPixel library helper functions for Esp8266 UART hardware

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/

#ifdef ARDUINO_ARCH_ESP8266
#include "NeoEsp8266UartMethod.h"
extern "C"
{
    #include <eagle_soc.h>
    #include <uart.h>
    #include <uart_register.h>
}

#define UART1 1
#define UART1_INV_MASK (0x3f << 19)

NeoEsp8266UartMethodBase::NeoEsp8266UartMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, uint32_t byteSendTimeUs)
{
    _sizePixels = pixelCount * elementSize;
    _pixelTime = byteSendTimeUs * _sizePixels + 50;
    _pixels = (uint8_t*)malloc(_sizePixels);
    memset(_pixels, 0x00, _sizePixels);
}

NeoEsp8266UartMethodBase::~NeoEsp8266UartMethodBase() throw()
{
    free(_pixels);
}

void NeoEsp8266UartMethodBase::InitializeUart(uint32_t uartBaud) throw()
{
    // Configure the serial line with 1 start bit (0), 6 data bits and 1 stop bit (1)
    Serial1.begin(uartBaud, SERIAL_6N1, SERIAL_TX_ONLY);

    // Invert the TX voltage associated with logic level so:
    //    - A logic level 0 will generate a Vcc signal
    //    - A logic level 1 will generate a Gnd signal
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART1_INV_MASK);
    SET_PERI_REG_MASK(UART_CONF0(UART1), (BIT(22)));

    // Inverting logic levels can generate a phantom bit in the led strip bus
    // We need to delay 50+ microseconds the output stream to force a data
    // latch and discard this bit. Otherwise, that bit would be prepended to
    // the first frame corrupting it.
    _startTime = micros() - _pixelTime + 50;
}

void NeoEsp8266UartMethodBase::Update() throw()
{
    // Data latch = 50+ microsecond pause in the output stream.  Rather than
    // put a delay at the end of the function, the ending time is noted and
    // the function will simply hold off (if needed) on issuing the
    // subsequent round of data until the latch time has elapsed.  This
    // allows the mainline code to start generating the next frame of data
    // rather than stalling for the latch
    while (!IsReadyToUpdate())
    {
        yield();
    }

    // since uart is async buffer send, we have to calc the endtime that it will take
    // to correctly manage the data latch in the above code
    // add the calculated time to the current time
    _startTime = micros();

    // esp hardware uart sending of data
    const uint8_t* ptr = _pixels;
    const uint8_t* end = ptr + _sizePixels;
    while (ptr != end)
    {
        ptr = FillUartFifo(ptr, end);
    }
}

const uint8_t* ICACHE_RAM_ATTR NeoEsp8266UartMethodBase::FillUartFifo(const uint8_t* pixels, const uint8_t* end) throw()
{
    // Remember: UARTs send less significant bit (LSB) first so
    //      sending 0bABCDEF will generate a 0FEDCBA1, including
    //      start & stop bits.
    // Also, we have configured UART to invert logic levels, so:
    const uint8_t _uartData[4] = {
        0b110111, // On wire: 1 000 100 0
        0b000111, // On wire: 1 000 111 0
        0b110100, // On wire: 1 110 100 0
        0b000100, // On wire: 1 110 111 0
    };
    unsigned avail = (UART_TX_FIFO_SIZE - ((U1S >> USTXC) & 0xff)) / 4;
    if (end - pixels > avail)
    {
        end = pixels + avail;
    }
    while (pixels < end)
    {
        uint8_t subpix = *pixels++;
        U1F = _uartData[(subpix >> 6) & 3];
        U1F = _uartData[(subpix >> 4) & 3];
        U1F = _uartData[(subpix >> 2) & 3];
        U1F = _uartData[subpix & 3];
    }
    return pixels;
}

#endif
