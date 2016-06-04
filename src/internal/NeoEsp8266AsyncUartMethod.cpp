/*-------------------------------------------------------------------------
NeoPixel library helper functions for Esp8266.

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

#ifdef ARDUINO_ARCH_ESP8266

#include "NeoEsp8266AsyncUartMethod.h"
#include <utility>
extern "C"
{
    #include <eagle_soc.h>
    #include <ets_sys.h>
    #include <uart.h>
    #include <uart_register.h>
}

static const uint8_t* esp8266_uart1_async_buf;
static const uint8_t* esp8266_uart1_async_buf_end;

NeoEsp8266AsyncUartMethodBase::NeoEsp8266AsyncUartMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, uint32_t byteSendTimeUs)
    : NeoEsp8266UartMethodBase(pin, pixelCount, elementSize, byteSendTimeUs)
{
    _asyncPixels = (uint8_t*)malloc(this->_sizePixels);
}

NeoEsp8266AsyncUartMethodBase::~NeoEsp8266AsyncUartMethodBase() throw()
{
    // Remember: the UART interrupt can be sending data from _asyncPixels in the background
    // We must wait until all data has been send
    while (!this->IsReadyToUpdate())
    {
        yield();
    }
    free(_asyncPixels);
}

void ICACHE_RAM_ATTR NeoEsp8266AsyncUartMethodBase::InitializeUart(uint32_t uartBaud) throw()
{
    NeoEsp8266UartMethodBase::InitializeUart(uartBaud);

    // Disable all interrupts
    ETS_UART_INTR_DISABLE();

    // Clear the RX & TX FIFOS
    SET_PERI_REG_MASK(UART_CONF0(UART1), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_RXFIFO_RST | UART_TXFIFO_RST);

    // Set the interrupt handler
    ETS_UART_INTR_ATTACH(IntrHandler, NULL);

    // Set tx fifo trigger. 80 bytes gives us 200 microsecs to refill the FIFO
    WRITE_PERI_REG(UART_CONF1(UART1), 80 << UART_TXFIFO_EMPTY_THRHD_S);

    // Disable RX & TX interrupts. It is enabled by uart.c in the SDK
    CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_RXFIFO_FULL_INT_ENA | UART_TXFIFO_EMPTY_INT_ENA);

    // Clear all pending interrupts in UART1
    WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);

    // Reenable interrupts
    ETS_UART_INTR_ENABLE();
}

void NeoEsp8266AsyncUartMethodBase::Update() throw()
{
    // Data latch = 50+ microsecond pause in the output stream.  Rather than
    // put a delay at the end of the function, the ending time is noted and
    // the function will simply hold off (if needed) on issuing the
    // subsequent round of data until the latch time has elapsed.  This
    // allows the mainline code to start generating the next frame of data
    // rather than stalling for the latch.
    while (!this->IsReadyToUpdate())
    {
        yield();
    }

    // Instruct ESP8266 hardware uart1 to send the pixels asynchronously
    esp8266_uart1_async_buf = this->_pixels;
    esp8266_uart1_async_buf_end = this->_pixels + this->_sizePixels;
    SET_PERI_REG_MASK(UART_INT_ENA(1), UART_TXFIFO_EMPTY_INT_ENA);

    // Annotate when we started to send bytes, so we can calculate when we are ready to send again
    this->_startTime = micros();

    // Copy the pixels to the idle buffer and swap them
    memcpy(_asyncPixels, this->_pixels, this->_sizePixels);
    std::swap(_asyncPixels, this->_pixels);
}

void ICACHE_RAM_ATTR NeoEsp8266AsyncUartMethodBase::IntrHandler(void* param) throw()
{
    // Interrupt handler is shared between UART0 & UART1
    if(READ_PERI_REG(UART_INT_ST(UART1)))   //any UART1 stuff
    {
        // Fill the FIFO with new data
        esp8266_uart1_async_buf = FillUartFifo(esp8266_uart1_async_buf, esp8266_uart1_async_buf_end);
        // Disable TX interrupt when done
        if (esp8266_uart1_async_buf == esp8266_uart1_async_buf_end)
        {
            CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);
        }
        // Clear all interrupts flags (just in case)
        WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);
    }

    if(READ_PERI_REG(UART_INT_ST(UART0)))
    {
        // TODO: gdbstub uses the interrupt of UART0, but there is no way to call its
        // interrupt handler gdbstub_uart_hdlr since it's static.
        WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
    }
}

#endif
