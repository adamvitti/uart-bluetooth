/**
 * @file leuart.c
 * @author Adam Vitti
 * @date 11/7/2021
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t  rx_done_evt;
uint32_t  tx_done_evt;
bool    leuart0_tx_busy;
static LEUART_STATE_MACHINE leuart0_state_machine;

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *   LEUART state machine function that services the TXBL interrupt
 *
 * @details
 *   This function will continuously transmit byte by byte through the LEUART peripheral until all data is sent.
 *   Each byte will be written to the TXDATA register until every byte has been written.
 *
 * @param[in] *leuart_sm
 *   Pointer to the state machine that contains variables relating to the state and data to be written
 *
 ******************************************************************************/
static void write_data_func(LEUART_STATE_MACHINE *leuart_sm){
  switch(leuart_sm->state){
    case write_UART:
      //write logic
      if(leuart_sm->count != leuart_sm->length){ // More data to write
          //write a byte
          leuart_app_transmit_byte(leuart_sm->leuart, leuart_sm->data[leuart_sm->count]); //Send one char/byte at a time
          leuart_sm->count++;
          break;
      }else{ //done writing data
          leuart_sm->leuart->IEN &= ~LEUART_IEN_TXBL;
          leuart_sm->state = end;
          leuart_sm->leuart->IEN |= LEUART_IEN_TXC;
          break;
      }
    case end: //should not get here
      EFM_ASSERT(false);
      break;
    default: //should not get here
      EFM_ASSERT(false);
      break;

  }
}

/***************************************************************************//**
 * @brief
 *   LEUART state machine function that services the TXC interrupt
 *
 * @details
 *   This function will be called at completion of data transmission. It will reset the state and LEUART peripheral, as well as, unblock energy modes.
 *
 * @param[in] *leuart_sm
 *   Pointer to the state machine that contains variables relating to the state and data to be written
 *
 ******************************************************************************/
static void stop_func(LEUART_STATE_MACHINE *leuart_sm){
  switch(leuart_sm->state){
    case write_UART://should not get here
      EFM_ASSERT(false);
      break;
    case end:
      //end logic
      leuart_sm->leuart->IEN &= ~LEUART_IEN_TXC;
      leuart_sm->state = write_UART;
      leuart_sm->available = true;
      sleep_unblock_mode(LEUART_TX_EM);
      add_scheduled_event(leuart_sm->leuart_cb); //service after uart write completes
      break;
    default: //should not get here
      EFM_ASSERT(false);
      break;

  }
}

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *  LEUART function that will initialize all parameters that are required for leuart0 operation
 *
 * @details
 *  This function enables the LEUART0 clock, routes the tx and rx pins, initializes all the leuart values that are needed for
 *  leuart0 operation, and enables leuart0 interrupts.
 *
 * @param[in] *leuart
 *  LEUART peripheral type define (LEUART0)
 *
 * @param[in] *leuart_settings
 *  This parameter is of type LEUART_OPEN_STRUCT and contains all the values to initialize the LEUART peripheral
 *
 ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
  LEUART_Init_TypeDef leuart_values;

  // Enables clock
  CMU_ClockEnable(cmuClock_LEUART0, true);

  //Test clock operation
  leuart->STARTFRAME = true; //write something to register to trigger write to RX register
  while(leuart->SYNCBUSY); //wait for write operation to RX, false after send and sync
  EFM_ASSERT(leuart->STARTFRAME & true); //makes sure the frame was actually sent (should be false & true)
  leuart->STARTFRAME = false; // send a 0 frame (false)
  while(leuart->SYNCBUSY); //wait for write operation to RX, false after send and sync

//  if ((leuart->IF & LEUART_IF_STARTF)) {
//      i2c->IFS = 0x01;
//      EFM_ASSERT(i2c->IF & 0x01);
//      i2c->IFC = 0x01;
//  } else {
//    i2c->IFC = 0x01;
//    EFM_ASSERT(!(i2c->IF & 0x01));
//  }
  LEUART_Enable_TypeDef disabled = leuartDisable;
  // Set initial leuart values for INIT
  leuart_values.refFreq = leuart_settings->refFreq;
  leuart_values.baudrate = leuart_settings->baudrate;
  leuart_values.databits = leuart_settings->databits;
  leuart_values.parity = leuart_settings->parity;
  leuart_values.stopbits = leuart_settings->stopbits;
  leuart_values.enable = disabled;
  leuart0_state_machine.available = true;

  tx_done_evt = leuart_settings->tx_done_evt;
  rx_done_evt = leuart_settings->rx_done_evt;

  LEUART_Init(leuart, &leuart_values);
  while(leuart->SYNCBUSY) //wait till init finishes

  leuart->ROUTELOC0 = leuart_settings->tx_loc | leuart_settings->rx_loc;

  leuart->ROUTEPEN |= (LEUART_ROUTEPEN_RXPEN * leuart_settings->rx_pin_en);
  leuart->ROUTEPEN |= (LEUART_ROUTEPEN_TXPEN * leuart_settings->tx_pin_en);

  leuart->STARTFRAME = LEUART_RXDATA_RXDATA_DEFAULT; //reset RX register
  while(leuart->SYNCBUSY);
  leuart->TXDATA = LEUART_TXDATA_TXDATA_DEFAULT; // reset TX register

  LEUART_Enable(leuart, leuart_settings->enable);

  while(!(leuart->STATUS & LEUART_STATUS_TXENS));
  while(!(leuart->STATUS & LEUART_STATUS_RXENS));
  EFM_ASSERT((leuart->STATUS & LEUART_STATUS_TXENS)); //checks that tx is running

  //Enable interrupts (going to be enabled later)
  //leuart->IEN |= (LEUART_IEN_TXC * leuart_settings->txc_irq_enable);
  //leuart->IEN |= (LEUART_IEN_TXBL * leuart_settings->txbl_irq_enable);
  //clear all interrupts
  leuart->IFC = _LEUART_IFC_MASK;

  NVIC_EnableIRQ(LEUART0_IRQn);

}

/***************************************************************************//**
 * @brief
 *  LEUART function that will be called for every leuart operation
 *
 * @details
 *  This function will initialize the leuart state machine struct that will be passed and used in leuart operation. It then
 *  enables the TXBL interrupt, allowing for transmit operations. Therefore, this function will be called for every operation of leuart.
 *
 * @param[in] *leuart
 *  LEUART peripheral type define (LEUART0)
 *
 * @param[in] *string
 *  This parameter will be the string of data to be transmitted through leuart.
 *
 *  @param[in] string_len
 *  Length of the string parameter
 *
 ******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
  while(!leuart0_state_machine.available);
  while(leuart->SYNCBUSY);

  CORE_DECLARE_IRQ_STATE; //atomic state
  CORE_ENTER_CRITICAL();

  leuart0_state_machine.available = false;
  sleep_block_mode(LEUART_TX_EM);

  leuart0_state_machine.leuart = leuart;
  leuart0_state_machine.length = string_len;
  leuart0_state_machine.count = 0;
  leuart0_state_machine.leuart_cb = tx_done_evt;
  leuart0_state_machine.state = write_UART;

  strcpy(leuart0_state_machine.data,string); //copy to struct



  leuart->IEN |= LEUART_IEN_TXBL; //allow interrupts for tx buffer
  CORE_EXIT_CRITICAL();

}


/***************************************************************************//**
 * @brief
 * Interrupt handler for the LEUART0 peripheral
 *
 * @details
 * This function handles the TXBL and TXC interrupts triggered within the leuart0 peripheral. It will call state machine functions to service the interrupt triggered based on its current state.
 *
 * @note
 * This function will respond and handle the TXBL and TXC interrupts.
 ******************************************************************************/

void LEUART0_IRQHandler(void){
  uint32_t int_flag = LEUART0->IF & LEUART0->IEN;
  LEUART0->IFC = int_flag;

  if(int_flag & LEUART_IF_TXBL){ // ready to send data
      write_data_func(&leuart0_state_machine);
  }
  if(int_flag & LEUART_IF_TXC){ // Transmission completed
      stop_func(&leuart0_state_machine);
  }
}


/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 *   This function enables the LEUART STATUS register to be provided to
 *   a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 *   Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
  uint32_t  status_reg;
  status_reg = leuart->STATUS;
  return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 *   This function is used by the TDD test function to program the LEUART
 *   for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 *   The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){
  leuart->CMD = cmd_update;
  while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 *   This function is used by the TDD test program to clear interrupts before
 *   the TDD tests and to reset the LEUART interrupts before the TDD
 *   exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
  leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 *   The BLE module will respond to AT commands if the BLE module is not
 *   connected to the phone app.  To validate the minimal functionality
 *   of the LEUART peripheral, write and reads to the LEUART will be
 *   performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
  while (!(leuart->IF & LEUART_IF_TXBL));
  leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 *   The BLE module will respond to AT commands if the BLE module is not
 *   connected to the phone app.  To validate the minimal functionality
 *   of the LEUART peripheral, write and reads to the LEUART will be
 *   performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 *   Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
  uint8_t leuart_data;
  while (!(leuart->IF & LEUART_IF_RXDATAV));
  leuart_data = leuart->RXDATA;
  return leuart_data;
}
