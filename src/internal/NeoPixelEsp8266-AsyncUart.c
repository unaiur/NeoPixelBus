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

#include <Arduino.h>
#include <eagle_soc.h>
#include <ets_sys.h>
#include <uart.h>
#include <uart_register.h>


static uint8_t const *esp8266_uart1_async_buf, *esp8266_uart1_async_buf_end;
uint8_t* ICACHE_RAM_ATTR esp8266_uart1_fill_fifo(uint8_t* pixels, uint8_t* end);

static void ICACHE_RAM_ATTR esp8266_uart1_intr_handler(void *para)
{
	/* uart0 and uart1 intr combine together, when interrupt occur, see reg 0x3ff20020, bit2, bit0 represents
	 * uart1 and uart0 respectively
	 */
	if(READ_PERI_REG(UART_INT_ST(UART1)))	//any UART1 stuff
	{
		// Fill the FIFO with new data
		esp8266_uart1_async_buf = esp8266_uart1_fill_fifo(esp8266_uart1_async_buf, esp8266_uart1_async_buf_end);
		// Disable tx_interrupt when done
		if (esp8266_uart1_async_buf == esp8266_uart1_async_buf_end)
			CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);
		WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);	//just to be sure :)
	}

	if(READ_PERI_REG(UART_INT_ST(UART0)))	//any UART0 stuff
	{
		// Currently, there is no way to call the uart0 irq that is used by gdbstub, since the interrupt handler is static
		// gdbstub_uart_hdlr(0);
		WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
	}
}

void ICACHE_RAM_ATTR esp8266_uart1_async_prepare()
{
	ETS_UART_INTR_DISABLE();

	//clear rx and tx fifo
	SET_PERI_REG_MASK(UART_CONF0(UART1), UART_RXFIFO_RST | UART_TXFIFO_RST);
	CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_RXFIFO_RST | UART_TXFIFO_RST);

	//overload uart int handler
	ETS_UART_INTR_ATTACH(esp8266_uart1_intr_handler,  NULL);

	//disable rx_interrupt. gets previously enables by uart.c
	CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_RXFIFO_FULL_INT_ENA);

	//set tx fifo trigger
	WRITE_PERI_REG(UART_CONF1(UART1), 80 << UART_TXFIFO_EMPTY_THRHD_S); //54: gives us 200uS to reakt

	//disable tx_interrupt
	CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);

	//clear all interrupt
	WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);

	ETS_UART_INTR_ENABLE();
}

void ICACHE_RAM_ATTR esp8266_uart1_async_send_pixels(uint8_t* pixels, uint8_t* end)
{
	esp8266_uart1_async_buf = pixels;
	esp8266_uart1_async_buf_end = end;
	SET_PERI_REG_MASK(UART_INT_ENA(1), UART_TXFIFO_EMPTY_INT_ENA);
}

#endif
