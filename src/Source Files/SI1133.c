/**
 * @file
 * SI1133.c
 * @author
 * Adam Vitti
 * @date
 * 10/16/21
 * @brief
 * This module initializes i2c operation with the si1133 peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "SI1133.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t si1133_read_data;
static uint32_t si1133_write_data;

//***********************************************************************************
// Private functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * This function configures si1133 for white light ADC operation
 *
 * @details
 * This function sends the proper parameter info and commands to setup the si1133 operation for white light ADC reading mode
 *
 * @note
 * This function will be called in the si1133 open func, in order to configure the si1133 operation
 *
 ******************************************************************************/
static void si1133_configure(){
  si1133_write_data = RESET_CMD_CNT;
  si1133_write(1,COMMAND,NULL_CB); //write our input data to INPUT0

  while(!i2c_available(I2C1)); //wait until end of i2c read

  si1133_read(1, RESPONSE0, NULL_CB); //expect 1 byte, response0 register, no callback
  while(!i2c_available(I2C1)); //wait until end of i2c read
  uint32_t cmd_ctr = si1133_read_data & 0x0f; //grab lower 4bits

  si1133_write_data = WHITE_LIGHT;
  si1133_write(1,INPUT0,NULL_CB); //write our input data to INPUT0

  while(!i2c_available(I2C1)); //wait until end of i2c write

  si1133_write_data = COMMAND_BITS | ADCCONFIG0;
  si1133_write(1,COMMAND,NULL_CB); //write the input0 data to adcconfig0 adcmux bits

  while(!i2c_available(I2C1)); //wait until end of i2c write

  // Verifies write command occurred
  si1133_read(1, RESPONSE0, NULL_CB); //expect 1 byte, response0 register, no callback
  while(!i2c_available(I2C1)); //wait until end of i2c read
  if((si1133_read_data & 0x0F) != cmd_ctr+1){
     EFM_ASSERT(false); //command write failed
  }

  si1133_write_data = CHANNEL0_PREP;
  si1133_write(1,INPUT0,NULL_CB); //write our input data to INPUT0

  while(!i2c_available(I2C1));

  si1133_write_data = COMMAND_BITS | CHAN_LIST;
  si1133_write(1,COMMAND,NULL_CB); //write the input0 data to chan_list

  while(!i2c_available(I2C1));

 // Verifies write command occurred
  si1133_read(1, RESPONSE0, NULL_CB); //expect 1 byte, response0 register, no callback
  while(!i2c_available(I2C1)); //wait until end of i2c read
  if((si1133_read_data & 0x0F) != cmd_ctr+2){
     EFM_ASSERT(false); //command write failed
  }

}


//***********************************************************************************
// Global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * This function initializes all i2c parameters for the si1133
 *
 * @details
 * This function passes a peripheral dependent struct to the general i2c driver in order to configure i2c to operate with the si1133 peripheral
 *
 * @note
 * This function will be called in app.c to setup i2c operation with the si1133.
 *
 ******************************************************************************/
void Si1133_i2c_open(){
  I2C_OPEN_STRUCT si113_i2c_open_struct;

  timer_delay(25); // 25ms for startup of sensor


  si113_i2c_open_struct.clhr = i2cClockHLRAsymetric; //6:3 ratio
  si113_i2c_open_struct.enable = true;
  si113_i2c_open_struct.freq = I2C_FREQ_FAST_MAX ; //400 Khz (si113 max freq)
  si113_i2c_open_struct.master = true;
  si113_i2c_open_struct.out_scl_en = true;
  si113_i2c_open_struct.out_sda_en = true;
  si113_i2c_open_struct.refFreq = 0; //gecko in master mode
  si113_i2c_open_struct.scl_out_route0 = I2C_SCL_PC5;
  si113_i2c_open_struct.sda_out_route0 = I2C_SDA_PC4;
  si113_i2c_open_struct.ack_irq_enable = true;
  si113_i2c_open_struct.rxdatav_irq_enable = true;
  si113_i2c_open_struct.stop_irq_enable = true;



  i2c_open(I2C1, &si113_i2c_open_struct);
  si1133_configure();
}


/***************************************************************************//**
 * @brief
 * This function uses i2c to read from the si1133 registers
 *
 * @details
 * This function calls the i2c_start() function with specific parameters related to the si1133 to read desired data
 *
 * @note
 * This function will be called in app.c to begin reading with i2c during automated clock cycles
 *
 * @param[in] bytes_expected
 * Sets the number of bytes to read from the si1133
 *
 * @param[in] desired_register_address
 * Sets the specific register to read out of the si1133
 *
 * @param[in] app_cb
 * Sets the callback function that will be serviced after a successful read operation
 *
 ******************************************************************************/
void si1133_read(uint32_t bytes_expected, uint32_t desired_register_address, uint32_t app_cb){
  uint32_t device_address = 0x55;

  i2c_start(I2C1, device_address, read, &si1133_read_data, bytes_expected, desired_register_address, app_cb);

}

/***************************************************************************//**
 * @brief
 * This function writes data to the si1133 peripheral through I2C
 *
 * @details
 * This function calls the i2c_start() function in "write" mode in order to write data to si1133 desired registers
 *
 * @note
 * This function will be called in app.c to begin writing with i2c during automated clock cycles
 *
 * @param[in] bytes_expected
 * Sets the number of bytes to write to the si1133
 *
 * @param[in] desired_register_address
 * Sets the specific register to write within the si1133
 *
 * @param[in] app_cb
 * Sets the callback function that will be serviced after a successful write operation
 *
 ******************************************************************************/
void si1133_write(uint32_t bytes_expected, uint32_t desired_register_address, uint32_t app_cb){
  uint32_t device_address = 0x55;

  i2c_start(I2C1, device_address, write, &si1133_write_data, bytes_expected, desired_register_address, app_cb);
}

/***************************************************************************//**
 * @brief
 * This function returns the read data from si1133
 *
 * @details
 * This function return the read data from a private static struct in order to access the data within another file
 *
 * @note
 * This function will be called within the callback function within app.c after a successful read operation
 *
 ******************************************************************************/
uint32_t si1133_read_result(){
  return si1133_read_data;
}

/***************************************************************************//**
 * @brief
 * This function will begin the ADC sampling of the si1133 peripheral
 *
 * @details
 * This function sends the FORCE command to the si1133 CMD register
 *
 * @note
 * This function will be called within the app.c timer comp1 callback function to be automatically called every period of the PWM
 *
 ******************************************************************************/
void si1133_force_cmd(){
//  si1133_read(1, RESPONSE0, NULL_CB); //expect 1 byte, response0 register, no callback
//  while(!i2c_available(I2C1)); //wait until end of i2c read
//  uint32_t cmd_ctr = si1133_read_data & 0x0f; //grab lower 4bits

  si1133_write_data = FORCE;
  si1133_write(1,COMMAND,NULL_CB); //write our input data to INPUT0
//
//  while(!i2c_available(I2C1));

//  // Verify write command
//  si1133_read(1, RESPONSE0, NULL_CB); //expect 1 byte, response0 register, no callback
//  while(!i2c_available(I2C1)); //wait until end of i2c read
//  if((si1133_read_data & 0x0f) != cmd_ctr+1){
//     EFM_ASSERT(false); //command write failed
//  }

}

/***************************************************************************//**
 * @brief
 * This function requests the white light ADC data from the si1133
 *
 * @details
 * This function begins reading at the HOSTOUT0 register for the first byte of data and will then read from HOST1, and so on, for subsequent data bytes
 *
 * @note
 * This function will be called within the callback function within app.c during the timer UF callback to ensure the si1133 has been properly configured and started
 *
 * @param light_cb
 * Sets the callback function that will be serviced after a successful white light read operation
 *
 ******************************************************************************/
void si1133_read_white_light(uint32_t light_cb){
  si1133_read(2, HOSTOUT0, light_cb);
}



