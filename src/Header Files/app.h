//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef APP_HG
#define APP_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"
#include <stdio.h>

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "scheduler.h"
#include "sleep_routines.h"
#include "LEDs_thunderboard.h"
#include "SI1133.h"
#include "ble.h"
#include "HW_Delay.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define   PWM_PER             2.0   // PWM period in seconds
#define   PWM_ACT_PER         .002  // PWM active period in seconds
#define   READ_BYTES          1     //Number of bytes we want to read from si1133
#define   EXPECTED_READ_DATA  20    //Part ID value expected to return from read
#define   SYSTEM_BLOCK_EM     EM3


//***********************************************************************************
// global variables
//***********************************************************************************
// Application scheduled events
#define   LETIMER0_COMP0_CB     0x00000001   //0b0001
#define   LETIMER0_COMP1_CB     0x00000002   //0b0010
#define   LETIMER0_UF_CB        0x00000004   //0b0100
#define   SI1133_LIGHT_CB       0x00000008   //0b1000
#define   BOOT_UP_CB            0x00000010  //0b10000
#define   BLE_TX_DONE_CB        0x00000020






//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void scheduled_letimer0_uf_cb (void);
void scheduled_letimer0_comp0_cb (void);
void scheduled_letimer0_comp1_cb (void);
void scheduled_si1133_read_cb(void);
void scheduled_boot_up_cb(void);
void scheduled_ble_tx_done_cb(void);
void rgb_led_open(void);

#endif
