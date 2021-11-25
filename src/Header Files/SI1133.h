/*
 * SI113.h
 *
 *  Created on: Oct 5, 2021
 *      Author: adamv
 */

#ifndef HEADER_FILES_SI1133_H_
#define HEADER_FILES_SI1133_H_

#include "i2c.h"
#include "brd_config.h"
#include "HW_delay.h"

#define   NULL_CB           0x00         //0b0000
#define   RESET_CMD_CNT     0x00
#define   PART_ID_REGISTER  0x00  //Register address for Part ID
#define   RESPONSE0         0x11
#define   INPUT0            0x0A
#define   COMMAND           0x0B
#define   ADCCONFIG0        0x02
#define   WHITE_LIGHT       0b01011 //For adcmux
#define   COMMAND_BITS      0b10000000 //for ORing with command
#define   CHANNEL0_PREP     0b000001
#define   CHAN_LIST         0x01
#define   FORCE             0x11 //force command
#define   HOSTOUT0          0x13
#define   HOSTOUT1          0x14
#define   HOSTOUT2          0x15

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void Si1133_i2c_open();
void si1133_read(uint32_t bytes_expected, uint32_t desired_register_address, uint32_t app_cb);
void si1133_write(uint32_t bytes_expected, uint32_t desired_register_address, uint32_t app_cb);
void si1133_force_cmd();
void si1133_read_white_light(uint32_t light_cb);
uint32_t si1133_read_result();

#endif /* HEADER_FILES_SI1133_H_ */
