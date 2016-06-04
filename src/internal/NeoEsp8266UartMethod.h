/*-------------------------------------------------------------------------
NeoPixel library helper functions for Esp8266.

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

#pragma once

#ifdef ARDUINO_ARCH_ESP8266
#include <Arduino.h>

class NeoEsp8266UartMethodBase
{
public:
    NeoEsp8266UartMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, uint32_t byteSendTimeUs);

    ~NeoEsp8266UartMethodBase() throw();

    bool IsReadyToUpdate() const
    {
        uint32_t delta = micros() - _startTime;
        return delta >= _pixelTime;
    }

    void InitializeUart(uint32_t uartBaud) throw();

    void Update() throw();

    uint8_t* getPixels() const
    {
        return _pixels;
    };

    size_t getPixelsSize() const
    {
        return _sizePixels;
    };

protected:
    static const uint8_t* ICACHE_RAM_ATTR FillUartFifo(const uint8_t* pixels, const uint8_t* end) throw();

    size_t    _sizePixels;   // Size of '_pixels' buffer below
    uint8_t* _pixels;        // Holds LED color values
    uint32_t _startTime;     // Microsecond count when last update started
    uint32_t _pixelTime;     // Time needed to send all the pixels and the data latch code
};

class NeoEsp8266UartSpeed800Kbps
{
public:
    static const uint32_t ByteSendTimeUs =  10; // us it takes to send a single pixel element at 800mhz speed
    static const uint32_t UartBaud = 3200000; // 800mhz, 4 serial bytes per NeoByte
};

class NeoEsp8266UartSpeed400Kbps
{
public:
    static const uint32_t ByteSendTimeUs = 20; // us it takes to send a single pixel element at 400mhz speed
    static const uint32_t UartBaud = 1600000; // 400mhz, 4 serial bytes per NeoByte
};

template<typename T_SPEED, typename T_BASE>
struct NeoEsp8266UartMethod: public T_BASE
{
    NeoEsp8266UartMethod(uint8_t pin, uint16_t pixelCount, size_t elementSize)
        : T_BASE(pin, pixelCount, elementSize, T_SPEED::ByteSendTimeUs)
    {
    }

    void Initialize()
    {
        this->InitializeUart(T_SPEED::UartBaud);
    }
};

typedef NeoEsp8266UartMethod<NeoEsp8266UartSpeed800Kbps, NeoEsp8266UartMethodBase> NeoEsp8266Uart800KbpsMethod;
typedef NeoEsp8266UartMethod<NeoEsp8266UartSpeed400Kbps, NeoEsp8266UartMethodBase> NeoEsp8266Uart400KbpsMethod;

#endif
