/*
 * i2c.h
 *
 *  Created on: Sep 28, 2021
 *      Author: adamv
 */

#ifndef I2C_HG
#define I2C_HG

/* Silicon Labs include statements */
#include "em_i2c.h"
#include "em_cmu.h"
#include <stdbool.h>
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// global variables
//***********************************************************************************
#define I2C_EM_BLOCK   EM2

typedef struct {
  bool                  enable;
  bool                  master;
  uint32_t              refFreq;
  uint32_t              freq;
  I2C_ClockHLR_TypeDef  clhr;
  uint32_t              scl_out_route0;  //scl and sda routes
  uint32_t              sda_out_route0;
  bool                  out_sda_en;   // enable out sda route
  bool                  out_scl_en;   // enable out scl route
  bool                  ack_irq_enable;
  bool                  rxdatav_irq_enable;
  bool                  stop_irq_enable;


} I2C_OPEN_STRUCT ;

typedef enum{
   write = 0,
   read = 1
}
OPERATION_MODE;

typedef enum {
  initialize_device_write,
  write_desired_register,
  initialize_device_read,
  write_data,
  recieve_data,
  stop
}
DEFINED_STATES;

typedef struct {
  I2C_TypeDef           *i2cx;
  uint32_t              device_address; //current device communicating on I2C
  bool                  available;
  OPERATION_MODE        mode; //read or write operation
  uint32_t              num_of_data_bytes;
  uint32_t              desired_register_address;
  uint32_t              *data;
  uint32_t              I2C_CB;
  DEFINED_STATES        current_state;

} I2C_STATE_MACHINE;


//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_start(I2C_TypeDef *i2c, uint32_t device_address, OPERATION_MODE mode, uint32_t *data, uint32_t bytes_expected, uint32_t desired_register_address, uint32_t app_cb);
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
bool i2c_available(I2C_TypeDef *i2c);
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);


#endif /* I2C_HG */
