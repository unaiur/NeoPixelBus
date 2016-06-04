/*-------------------------------------------------------------------------
NeoPixel library helper functions for using asynchronous UART1 in Esp8266.

Written by Unai Uribarri

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

#pragma once

#ifdef ARDUINO_ARCH_ESP8266
#include "NeoEsp8266UartMethod.h"

// due to linker overriding ICACHE_RAM_ATTR for cpp files, this function was moved into a NeoPixelEsp8266-AsyncUart.c file.
extern "C" {
    void ICACHE_RAM_ATTR esp8266_uart1_async_prepare();
    void ICACHE_RAM_ATTR esp8266_uart1_async_send_pixels(uint8_t* pixels, uint8_t* end);
}

template<typename T_SPEED> class NeoEsp8266AsyncUartMethodBase: public NeoEsp8266UartMethodBase<T_SPEED>
{
public:
    NeoEsp8266AsyncUartMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize)
	: NeoEsp8266UartMethodBase<T_SPEED>(pin, pixelCount, elementSize)
    {
        _asyncPixels = (uint8_t*)malloc(this->_sizePixels);
    }

    ~NeoEsp8266AsyncUartMethodBase()
    {
        while (!this->IsReadyToUpdate())
            yield();
        free(_asyncPixels);
    }

    void Initialize()
    {
	NeoEsp8266UartMethodBase<T_SPEED>::Initialize();
	esp8266_uart1_async_prepare();
    }

    void Update()
    {
        // Data latch = 50+ microsecond pause in the output stream.  Rather than
        // put a delay at the end of the function, the ending time is noted and
        // the function will simply hold off (if needed) on issuing the
        // subsequent round of data until the latch time has elapsed.  This
        // allows the mainline code to start generating the next frame of data
        // rather than stalling for the latch.
        while (!this->IsReadyToUpdate())
            yield();

        // Instruct ESP8266 hardware uart1 to send the pixels asynchronously
        esp8266_uart1_async_send_pixels(this->_pixels, this->_pixels + this->_sizePixels);

        // Annotate when we started to send bytes, so we can calculate when we are ready to send again
        this->_startTime = micros();

	// Copy the pixels to the idle buffer and swap them
	memcpy(_asyncPixels, this->_pixels, this->_sizePixels);
	std::swap(_asyncPixels, this->_pixels);
    }

private:
    uint8_t* _asyncPixels;  // Holds a copy of LED color values when Update began
};

typedef NeoEsp8266AsyncUartMethodBase<NeoEsp8266UartSpeed800Kbps> NeoEsp8266AsyncUart800KbpsMethod;
typedef NeoEsp8266AsyncUartMethodBase<NeoEsp8266UartSpeed400Kbps> NeoEsp8266AsyncUart400KbpsMethod;

#endif
