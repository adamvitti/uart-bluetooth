/**
 * @file
 * gpio.c
 * @author
 * Adam Vitti
 * @date
 * 10/16/21
 * @brief
 * This module configures and implements state machine logic for i2c operation
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"

//***********************************************************************************
// Private Variables
//***********************************************************************************
static I2C_STATE_MACHINE i2c0_state, i2c1_state;

//***********************************************************************************
// Private functions
//***********************************************************************************
void i2c_bus_reset(I2C_TypeDef *i2c);

/***************************************************************************//**
 * @brief
 * This state machine function services ACK interrupts
 *
 * @details
 * Once the mighty gecko receives an ACK from the i2c peripheral, it will execute 1 of 3 operations depending on its state machine current state.
 * If the current state is "initialize_device_data" it will then execute the logic to specify the register desired for data and determine whether this is a "read" or "write" operation and change the state accordingly.
 * If the current state is "write_desired_register" we have already executed the logic specified before, and will then execute the logic in order to read from the device
 * If the current state is "write_data," indicating a "write" operation, the logic will write data to the TXDATA register to be transmitted.
 * If the current state is neither of the previously mentioned states, the function will throw an EFM ASSERT false because we should never have an ACK within the other states.
 * After the completion of each logic specified, the state machine state will be incremented to the next state.
 *
 * @note
 * This function is called within the i2c interrupt request handler if an ACK bit is set within the interrupt flag register
 *
 ******************************************************************************/
static void Ack_Func(I2C_STATE_MACHINE *i2c_sm){
  switch (i2c_sm->current_state){
    case initialize_device_write:
      i2c_sm->i2cx->TXDATA = i2c_sm->desired_register_address;

      if(i2c_sm->mode == read){
          i2c_sm->current_state = write_desired_register;
      }else if(i2c_sm->mode == write){
          i2c_sm->current_state = write_data;
      }
      break;
    case write_desired_register:
      i2c_sm->i2cx->CMD = I2C_CMD_START;
      i2c_sm->i2cx->TXDATA = (i2c_sm->device_address << 1) | read ;
      i2c_sm->current_state = initialize_device_read;
      break;
    case initialize_device_read:
      break;
    case write_data:
      i2c_sm->num_of_data_bytes--;
      i2c_sm->i2cx->TXDATA = (*(i2c_sm->data) >> (8*i2c_sm->num_of_data_bytes)) & 0xff;
      if(i2c_sm->num_of_data_bytes == 0){
         i2c_sm->i2cx->CMD = I2C_CMD_STOP;
         i2c_sm->current_state = recieve_data;
         break;
     }
     break;
    case recieve_data:
      break;
    case stop:
    default:
      EFM_ASSERT(false);
      break;
  }

}

/***************************************************************************//**
 * @brief
 * This state machine function services RXDATAV interrupts
 *
 * @details
 * Once the mighty gecko sees receive data available from the i2c peripheral, it will execute logic to read data from the peripherals specified register.
 * The logic will read a single byte from the RXDATAV register and send an ACK to the peripheral if more data is expected for read. The state machine state will not be incremented,
 * keeping the read data operation in a loop until the number of bytes expected are read from the peripheral. At this point the logi will send a NACK and STOP command indicating the
 * read operation is complete, and then increment the state machine state.
 * If the current state is not in the "initialize_device_read" state, the function will throw an EFM ASSERT false because we should never have RXDATAV within the other states.
 *
 * @note
 * This function is called within the i2c interrupt request handler if a RXDATAV bit is set within the interrupt flag register
 *
 ******************************************************************************/
static void Rxdatav_Func(I2C_STATE_MACHINE *i2c_sm){
  switch (i2c_sm->current_state){
      case initialize_device_write:
        EFM_ASSERT(false);
        break;
      case write_desired_register:
        EFM_ASSERT(false);
        break;
      case initialize_device_read:
            i2c_sm->num_of_data_bytes--;
            *(i2c_sm->data) &= ~(0xff << (8*i2c_sm->num_of_data_bytes));
            *(i2c_sm->data) |= i2c_sm->i2cx->RXDATA << (8*i2c_sm->num_of_data_bytes);
            if(i2c_sm->num_of_data_bytes > 0){ //still have more data to read
                i2c_sm->i2cx->CMD = I2C_CMD_ACK;
                break;
            }else{ //done reading data
                i2c_sm->i2cx->CMD = I2C_CMD_NACK;
                i2c_sm->i2cx->CMD = I2C_CMD_STOP;
                i2c_sm->current_state = recieve_data;
                break;
            }
      case recieve_data:
      case stop:
      default:
        EFM_ASSERT(false);
        break;
    }

}

/***************************************************************************//**
 * @brief
 * This state machine function services MSTOP interrupts
 *
 * @details
 * This function will verify that a stop condition has been sent along the i2c peripheral. The only way this function is called is when the MSTOP bit within the IF
 * register is set, so the act of entering this function verifies the stop condition. Once it is known that i2c operation is complete, the mighty gecko energy modes
 * will be unblocked and the i2c peripheral will be free to perform other i2c operations. Additionally, at the end of the i2c operation, an event will be scheduled to
 * pass the received data up to application code. The state machine will then be reset to its initial state.
 * If the current state is not in the "receive_data" state, the function will throw an EFM ASSERT false because we should never have an MSTOP within the other states.
 *
 * @note
 * This function is called within the i2c interrupt request handler if a MSTOP bit is set within the interrupt flag register
 *
 ******************************************************************************/
static void Stop_Func(I2C_STATE_MACHINE *i2c_sm){
  switch (i2c_sm->current_state){
        case initialize_device_write:
          EFM_ASSERT(false);
          break;
        case write_desired_register:
          EFM_ASSERT(false);
          break;
        case initialize_device_read:
          EFM_ASSERT(false);
          break;
        case recieve_data:
          //Only get to this point if MSTOP was set in IRQ Handler
          //unblock sleep mode after verifying stop
              sleep_unblock_mode(I2C_EM_BLOCK);
              i2c_sm->available = true;
              i2c_sm->current_state = initialize_device_write;
              add_scheduled_event(i2c_sm->I2C_CB);
          break;
        case stop:
        default:
          EFM_ASSERT(false);
          break;
      }
}


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Begins i2c read/write operations
 *
 * @details
 * This function will start either the I2C0 or I2C1 peripheral with the specified arguments in the function call.
 *
 * @note
 * This function will be used in a sensor/peripheral driver to read or write data to a device by using the function parameters
 *
 * @param[in] i2c
 * A pointer/address to the desired i2c peripheral to be used in operation
 *
 * @param[in] device_address
 * Address of the slave peripheral to communicate with over i2c
 *
 * @param[in] mode
 * Operation mode of the i2c peripheral, either "write" or "read"
 *
 * @param[in] data
 * A pointer to the data variable that will hold the data read off or written to the slave device
 *
 * @param[in] bytes_expected
 * Number of bytes desired to read off or write to the slave device
 *
 * @param[in] desired_register_address
 * Address of the slave register desired to read or write to
 *
 * @param[in] app_cb
 * Call back function to be serviced after i2c operation completes
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2c, uint32_t device_address, OPERATION_MODE mode, uint32_t *data, uint32_t bytes_expected, uint32_t desired_register_address, uint32_t app_cb){ //input number of bytes wanting to read
  I2C_STATE_MACHINE *i2c_local_sm = 0;

  if(i2c == I2C0){
      i2c_local_sm = &i2c0_state;
  }else if(i2c == I2C1){
      i2c_local_sm = &i2c1_state;
  }else{
      EFM_ASSERT(false);
  }
  while(!i2c_local_sm->available);

  EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);

  sleep_block_mode(I2C_EM_BLOCK); //block energy modes > 2


  i2c_local_sm->available = false;

  // initialize struct
  i2c_local_sm->i2cx = i2c;
  i2c_local_sm->mode = mode;
  i2c_local_sm->I2C_CB = app_cb;
  i2c_local_sm->data = data;
  i2c_local_sm->num_of_data_bytes = bytes_expected;
  i2c_local_sm->current_state = initialize_device_write; //initial state 0
  i2c_local_sm->desired_register_address = desired_register_address;
  i2c_local_sm->device_address = device_address;

  i2c->CMD = I2C_CMD_START;
  i2c->TXDATA = (device_address << 1) | write;

}


/***************************************************************************//**
 * @brief
 * Initializes i2c peripherals
 *
 * @details
 * This function initializes either the I2C0 or I2C1 peripheral clocks and sets all necessary values for setup.
 * It then routes the i2c peripheral to the desired sensor/device, and enables interrupts
 *
 * @note
 * This function is called once in app_peripheral_setup() in order to setup i2c in the operation mode
 * specific to this application.
 *
 * @param[in] i2c
 * A pointer/address to the desired i2c peripheral to be initialized
 *
 * @param[in] i2c_setup
 * A struct that contains all the app specific values for initialization:
 * 1. Clock low/high ratio
 * 2. Frequency of operation
 * 3. Master bit
 * 4. Reference frequency
 * 5. Enable bit
 * As well as route locations for the peripheral.
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup){
  I2C_Init_TypeDef i2c_values;

  // Enables clock
  if(i2c == I2C0){
      CMU_ClockEnable(cmuClock_I2C0, true);
      i2c0_state.available = true;
  }
  if(i2c == I2C1){
      CMU_ClockEnable(cmuClock_I2C1, true);
      i2c1_state.available = true;
    }

  // Test clock operation
  if ((i2c->IF & 0x01) == 0) {
      i2c->IFS = 0x01;
      EFM_ASSERT(i2c->IF & 0x01);
      i2c->IFC = 0x01;
  } else {
    i2c->IFC = 0x01;
    EFM_ASSERT(!(i2c->IF & 0x01));
  }

  // Set initial i2c values for INIT
  i2c_values.clhr = i2c_setup->clhr;
  i2c_values.freq = i2c_setup->freq;
  i2c_values.master = i2c_setup->master;
  i2c_values.refFreq = i2c_setup->refFreq;
  i2c_values.enable = i2c_setup->enable;

  I2C_Init(i2c, &i2c_values);

  // Route I2Cx to desired location
  i2c->ROUTELOC0 = i2c_setup->scl_out_route0 | i2c_setup->sda_out_route0;

  i2c->ROUTEPEN |= (I2C_ROUTEPEN_SDAPEN * i2c_setup->out_sda_en);
  i2c->ROUTEPEN |= (I2C_ROUTEPEN_SCLPEN * i2c_setup->out_scl_en);

  i2c->IEN |= (I2C_IEN_ACK * i2c_setup->ack_irq_enable);
  i2c->IEN |= (I2C_IEN_RXDATAV * i2c_setup->rxdatav_irq_enable);
  i2c->IEN |= (I2C_IEN_MSTOP * i2c_setup->stop_irq_enable);

  if(i2c == I2C0){
      NVIC_EnableIRQ(I2C0_IRQn);
  }
  if(i2c == I2C1){
      NVIC_EnableIRQ(I2C1_IRQn);
    }




  i2c_bus_reset(i2c);

}


/***************************************************************************//**
 * @brief
 * Resets the i2c bus
 *
 * @details
 * This function sends an abort command to stop any i2c operation, clears the interrupt flag register (after saving the interrupt state),
 * clears the transmit register, then sends a start and stop command simultaneously in order to reset the entire peripheral. After reset, the interrupt enable register
 * is restored to its previous state.
 *
 * @note
 * This function is called once in app_peripheral_setup() in order to setup i2c in the operation mode
 * specific to this application.
 *
 * @param[in] i2c
 * A pointer/address to the desired i2c peripheral to be reset
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef *i2c){
  i2c->CMD = I2C_CMD_ABORT;

  uint32_t savedState = i2c->IEN; //save state
  i2c->IEN = 0; //Disable all interrupts

  i2c->IFC = i2c->IF; //clear any interrupts that are flagged, as well as MSTOP
  i2c->CMD = I2C_CMD_CLEARTX; //clear transmit buffer
  i2c->CMD = (I2C_CMD_START | I2C_CMD_STOP); // perform restart

  while(!(i2c->IF & I2C_IF_MSTOP)); //Ensure restart was successful

  i2c->IFC = i2c->IF; //clear any interrupts that may have been triggered by restart

  i2c->CMD = I2C_CMD_ABORT;
  i2c->IEN = savedState; // restore state

}

/***************************************************************************//**
 * @brief
 * Returns the i2c peripheral availability state
 *
 * @details
 * This function will indicate whether or not the i2c peripheral is currently performing an operation or available for use, indicating all operations have completed.
 *
 * @note
 * This function will be used within si1133 config to determine if i2c operations have completed and data is valid and available.
 ******************************************************************************/
bool i2c_available(I2C_TypeDef *i2c){
  if(i2c == I2C0){
      return i2c0_state.available;
    }
  if(i2c == I2C1){
      return i2c1_state.available;
    }
  return false;
}

/***************************************************************************//**
 * @brief
 * Interrupt handler for the I2C0 peripheral
 *
 * @details
 * This function handles all interrupts triggered within the i2c0 peripheral. It will call state machine functions to service the interrupt triggered based on its current state.
 *
 * @note
 * This function will respond and handle the ACK, RXDATAV, and MSTOP interrupts
 ******************************************************************************/
void I2C0_IRQHandler(void){
  uint32_t int_flag = I2C0->IF & I2C0->IEN;
  I2C0->IFC = int_flag;

  if(int_flag & I2C_IF_ACK) {
      Ack_Func(&i2c0_state);
  }
  if(int_flag & I2C_IF_RXDATAV){
      Rxdatav_Func(&i2c0_state);
     }
  if(int_flag & I2C_IF_MSTOP){
      Stop_Func(&i2c0_state);
     }
}

/***************************************************************************//**
 * @brief
 * Interrupt handler for the I2C1 peripheral
 *
 * @details
 * This function handles all interrupts triggered within the i2c1 peripheral. It will call state machine functions to service the interrupt triggered based on its current state.
 *
 * @note
 * This function will respond and handle the ACK, RXDATAV, and MSTOP interrupts
 ******************************************************************************/
void I2C1_IRQHandler(void){
  uint32_t int_flag = I2C1->IF & I2C1->IEN;
   I2C1->IFC = int_flag;

   if(int_flag & I2C_IF_ACK) {
       Ack_Func(&i2c1_state);
   }
   if(int_flag & I2C_IF_RXDATAV){
       Rxdatav_Func(&i2c1_state);
   }
   if(int_flag & I2C_IF_MSTOP){
       Stop_Func(&i2c1_state);
      }


}




