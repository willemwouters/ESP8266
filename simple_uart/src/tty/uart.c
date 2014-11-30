/*
 * File	: uart.c
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ets_sys.h"
#include "espmissingincludes.h"
#include "user_interface.h"
#include "osapi.h"
#include "uart.h"
#include "osapi.h"
#include "uart_register.h"

//#include "ssc.h"
os_event_t    uart_readoutTaskQueue[uart_readoutQeueLen];
os_event_t    uart_processDataTaskQueue[uart_processdataQueueLen];

static char uart_receivebuffer[uart_receivebufferMax];

// UartDev is defined and initialized in rom code.
extern UartDevice    UartDev;

LOCAL void uart0_rx_intr_handler(void *para);


/******************************************************************************
 * FunctionName : uart_config
 * Description  : Internal used function
 *                UART0 used for data TX/RX, RX buffer size is 0x100, interrupt enabled
 *                UART1 just used for debug output
 * Parameters   : uart_no, use UART0 or UART1 defined ahead
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart_config(uint8 uart_no)
{
  if (uart_no == UART1)
  {
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
  }
  else
  {
    /* rcv_buff size if 0x100 */
    ETS_UART_INTR_ATTACH(uart0_rx_intr_handler,  &(UartDev.rcv_buff));
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_U0RTS);
  }

  uart_div_modify(uart_no, UART_CLK_FREQ / (UartDev.baut_rate));

  WRITE_PERI_REG(UART_CONF0(uart_no), UartDev.exist_parity
                 | UartDev.parity
                 | (UartDev.stop_bits << UART_STOP_BIT_NUM_S)
                 | (UartDev.data_bits << UART_BIT_NUM_S));

  //clear rx and tx fifo,not ready
  SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
  CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);

  if (uart_no == UART0)
  {
    //set rx fifo trigger
    WRITE_PERI_REG(UART_CONF1(uart_no),
                   ((0x10 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |
                   ((0x10 & UART_RX_FLOW_THRHD) << UART_RX_FLOW_THRHD_S) |
                   UART_RX_FLOW_EN |
                   (0x02 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S |
                   UART_RX_TOUT_EN);
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_TOUT_INT_ENA |
                      UART_FRM_ERR_INT_ENA);
  }
  else
  {
    WRITE_PERI_REG(UART_CONF1(uart_no),
                   ((UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S));
  }

  //clear all interrupt
  WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
  //enable rx_interrupt
  SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA);
}

/******************************************************************************
 * FunctionName : uart1_tx_one_char
 * Description  : Internal used function
 *                Use uart1 interface to transfer one char
 * Parameters   : uint8 TxChar - character to tx
 * Returns      : OK
*******************************************************************************/
LOCAL STATUS
uart_tx_one_char(uint8 uart, uint8 TxChar)
{
    while (true)
    {
      uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(uart)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
      if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
        break;
      }
    }

    WRITE_PERI_REG(UART_FIFO(uart) , TxChar);
    return OK;
}



/******************************************************************************
 * FunctionName : uart1_write_char
 * Description  : Internal used function
 *                Do some special deal while tx char is '\r' or '\n'
 * Parameters   : char c - character to tx
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart0_write_char(char c)
{
  if (c == '\n')
  {
    uart_tx_one_char(UART0, '\r');
    uart_tx_one_char(UART0, '\n');
  }
  else if (c == '\r')
  {
  }
  else
  {
    uart_tx_one_char(UART0, c);
  }
}
/******************************************************************************
 * FunctionName : uart0_tx_buffer
 * Description  : use uart0 to transfer buffer
 * Parameters   : uint8 *buf - point to send buffer
 *                uint16 len - buffer len
 * Returns      :
*******************************************************************************/
void ICACHE_FLASH_ATTR
uart0_tx_buffer(uint8 *buf, uint16 len)
{
  uint16 i;

  for (i = 0; i < len; i++)
  {
    uart_tx_one_char(UART0, buf[i]);
  }
}

/******************************************************************************
 * FunctionName : uart0_sendStr
 * Description  : use uart0 to transfer buffer
 * Parameters   : uint8 *buf - point to send buffer
 *                uint16 len - buffer len
 * Returns      :
*******************************************************************************/
void ICACHE_FLASH_ATTR
uart0_sendStr(const char *str)
{
	while(*str)
	{
		uart_tx_one_char(UART0, *str++);
	}
}

/******************************************************************************
 * FunctionName : uart0_rx_intr_handler
 * Description  : Internal used function
 *                UART0 interrupt handler, add self handle code inside
 * Parameters   : void *para - point to ETS_UART_INTR_ATTACH's arg
 * Returns      : NONE
*******************************************************************************/
//extern void at_recvTask(void);

LOCAL void
uart0_rx_intr_handler(void *para)
{
  uint8 uart_no = UART0;//UartDev.buff_uart_no;

  if(UART_FRM_ERR_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_FRM_ERR_INT_ST))
  {
    os_printf("FRM_ERR\r\n");
    WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_FRM_ERR_INT_CLR);
  }
  if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_FULL_INT_ST))
  {
    ETS_UART_INTR_DISABLE();
    system_os_post(uart_readoutTrigger, 0, 0); // trigger a readout of the buffer
  }
  else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_TOUT_INT_ST))
  {
    ETS_UART_INTR_DISABLE();
    system_os_post(uart_readoutTrigger, 0, 0); // trigger a readout of the buffer
  }

}

// Will handle the received data if \n was found, over and over again
static void ICACHE_FLASH_ATTR uart_readTask()
{
  if(uartrcv_state == uartrcv_statProcess)
  {
      uart0_sendStr("\r\n We RECEIVED A WHILE LINE OF DATA ===========\r\n");
      uart0_sendStr(uart_receivebuffer);
      uart0_sendStr("\r\n===========END OF DATA\r\n");
	  uartrcv_state = uartrcv_begin;
  }
}
#undef USE_PREFIX

static void ICACHE_FLASH_ATTR ///////
uart_recvTask(os_event_t *events)
{
#ifdef USE_PREFIX
  static uint8_t atHead[2];
#endif

  static char *pCmdLine;
  uint8_t temp;

  while(READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
  {
     WRITE_PERI_REG(0X60000914, 0x73); //WTD
	  temp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
	  if((temp != '\n'))
	  {
		uart_tx_one_char(UART0, temp); //display back
	  }
    switch(uartrcv_state)
    {
    case uartrcv_begin: //serch "AT" head
#ifdef USE_PREFIX
      atHead[0] = atHead[1]; // small shift buffer for first two letters
      atHead[1] = temp;
      if(os_memcmp(atHead, "AT", 2) == 0)
      {
#endif
    	  uartrcv_state = uartrcv_statRecving;
        pCmdLine = uart_receivebuffer; // set pointer to begin of buffer
#ifdef USE_PREFIX
        atHead[1] = 0x00;
      }
      else  if(temp == '\n') //only get enter
      {
        uart0_sendStr("\r\nERROR\r\n");
      }
#endif
      break;
    case uartrcv_statRecving: //push receive data to cmd line
      *pCmdLine = temp;
      if(temp == '\n') // found newline lets parse our data
      {
        system_os_post(uart_processdataTrigger, 0, 0); // buffer might still be full be full, thats why this is handled is seperate task
        pCmdLine++;
        *pCmdLine = '\0';
        uartrcv_state = uartrcv_statProcess;
        uart0_sendStr("\r\n");
      }
      else if(pCmdLine >= &uart_receivebuffer[uart_receivebufferMax - 1]) // we exeeded max length of line, reset
      {
    	  uartrcv_state = uartrcv_begin;
      }
      pCmdLine++;
      break;

    case uartrcv_statProcess: // We had found an \n, but buffer is full, and now we need to do something and then come back with buffer
      if(temp == '\n')
      {
        uart0_sendStr("\r\nbusy p...\r\n");
      }
      break;
    default:
      if(temp == '\n')
      {
      }
      break;
    }
  }
  if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
  {
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
  }
  else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
  {
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
  }
  ETS_UART_INTR_ENABLE();
}


/******************************************************************************
 * FunctionName : uart_init
 * Description  : user interface for init uart
 * Parameters   : UartBautRate uart0_br - uart0 bautrate
 *                UartBautRate uart1_br - uart1 bautrate
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
uart_init(UartBautRate uart0_br)
{
  // rom use 74880 baut_rate, here reinitialize
  UartDev.baut_rate = uart0_br;
  uart_config(UART0);

  ETS_UART_INTR_ENABLE();
  // install uart1 putc callback
  os_install_putc1((void *)uart0_write_char);

  system_os_task(uart_recvTask, uart_readoutTrigger, uart_readoutTaskQueue, uart_readoutQeueLen);
  system_os_task(uart_readTask, uart_processdataTrigger, uart_processDataTaskQueue, uart_processdataQueueLen);

}

void ICACHE_FLASH_ATTR
uart_reattach()
{
	uart_init(BIT_RATE_115200);
//  ETS_UART_INTR_ATTACH(uart_rx_intr_handler_ssc,  &(UartDev.rcv_buff));
//  ETS_UART_INTR_ENABLE();
}
