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

class NeoEsp8266AsyncUartMethodBase: public NeoEsp8266UartMethodBase
{
public:
    NeoEsp8266AsyncUartMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, uint32_t byteSendTimeUs);

    ~NeoEsp8266AsyncUartMethodBase() throw();

    void InitializeUart(uint32_t uartBaud) throw();

    void Update() throw();

private:
    static void ICACHE_RAM_ATTR IntrHandler(void* param) throw();

    uint8_t* _asyncPixels;  // Holds a copy of LED color values when Update began
};

typedef NeoEsp8266UartMethod<NeoEsp8266UartSpeed800Kbps, NeoEsp8266AsyncUartMethodBase> NeoEsp8266AsyncUart800KbpsMethod;
typedef NeoEsp8266UartMethod<NeoEsp8266UartSpeed400Kbps, NeoEsp8266AsyncUartMethodBase> NeoEsp8266AsyncUart400KbpsMethod;

#endif
