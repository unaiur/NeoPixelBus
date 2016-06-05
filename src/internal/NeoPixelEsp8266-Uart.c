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

#ifdef ARDUINO_ARCH_ESP8266

#include <Arduino.h>
#include <eagle_soc.h>
#include <uart.h>
#include <uart_register.h>

uint8_t* ICACHE_RAM_ATTR esp8266_uart1_fill_fifo(uint8_t* pixels, uint8_t* end)
{
    const uint8_t _uartData[4] = { 0b00110111, 0b00000111, 0b00110100, 0b00000100 };
    unsigned avail = (UART_TX_FIFO_SIZE - ((U1S >> USTXC) & 0xff)) / 4;
    if (end - pixels > avail)
        end = pixels + avail;
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
