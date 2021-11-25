/**
 * @file
 * letimer.c
 * @author
 * Adam Vitti
 * @date
 * 9/23/21
 * @brief
 * Module that handles available sleep/energy modes.
 *
 */

/**************************************************************************
 * @file sleep.c
 ***************************************************************************
 * @section License* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 * ***************************************************************************
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitnessfor any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 ****************************************************************************/
//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"



//***********************************************************************************
// Private variables
//***********************************************************************************
static int lowest_energy_modes[MAX_ENERGY_MODES];

//***********************************************************************************
// Global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * This function initializes the lowest energy mode array to 0.
 *
 * @details
 * This function ensures that the allowed energy modes are intialized
 *
 * @note
 * This function will be called in app.c once in order to initialize our sleep mode operations. It should be called before any peripherals
 * are started.
 *
 ******************************************************************************/
void sleep_open(void){
  for(int i = 0; i < MAX_ENERGY_MODES - 1; i++){
      lowest_energy_modes[i] = 0;
  }

}

/***************************************************************************//**
 * @brief
 * This function blocks energy mode operations.
 *
 * @details
 * This function will stop the processor from entering an incompatible sleep mode once a peripheral is running.
 *
 * @note
 * This function will be called in letimer.c to block sleep mode operation once the LETIMER0 begins running.
 *
 * @param[in] EM
 * The "EM" parameter is an integer that defines the energy mode we want to block.
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM){
  /* Atomic event */
  CORE_DECLARE_IRQ_STATE; //Save IRQ state
  CORE_ENTER_CRITICAL(); //disables interrupts and saves IEN bit
  lowest_energy_modes[EM]++;
  EFM_ASSERT (lowest_energy_modes[EM] < 5);
  CORE_EXIT_CRITICAL(); //Restores interrupt processes
}

/***************************************************************************//**
 * @brief
 * This function unblocks energy mode operations
 *
 * @details
 * This function will allow the processor to enter a sleep mode that was previously blocked.
 *
 * @note
 * This function will be called once a peripheral that blocked a sleep mode is no longer running.
 *
 * @param[in] EM
 * The "EM" parameter is an integer that defines the energy mode we want to unblock.
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM){
  /* Atomic event */
  CORE_DECLARE_IRQ_STATE; //Save IRQ state
  CORE_ENTER_CRITICAL(); //disables interrupts and saves IEN bit
  lowest_energy_modes[EM]--;
  EFM_ASSERT(lowest_energy_modes[EM] >= 0);
  CORE_EXIT_CRITICAL(); //Restores interrupt processes
}

/***************************************************************************//**
 * @brief
 * This function puts our processor into a sleep mode that is compatible with all peripherals that are running.
 *
 * @details
 * This function will put the processor into the lowest sleep mode available during operation.
 *
 * @note
 * The lowest energy modes array is used to determine which energy mode the processor can be put into.
 *
 ******************************************************************************/
void enter_sleep(void){
  /* Atomic event */
  CORE_DECLARE_IRQ_STATE; //Save IRQ state
  CORE_ENTER_CRITICAL(); //disables interrupts and saves IEN bit

  if(lowest_energy_modes[EM0] > 0){
      CORE_EXIT_CRITICAL(); //Restores interrupt processes
      return;
  }else if(lowest_energy_modes[EM1] > 0){
      CORE_EXIT_CRITICAL(); //Restores interrupt processes
      return;
  }else if(lowest_energy_modes[EM2] > 0){
      EMU_EnterEM1();
      CORE_EXIT_CRITICAL(); //Restores interrupt processes
      return;
  }else if(lowest_energy_modes[EM3] > 0){
      EMU_EnterEM2(true);
      CORE_EXIT_CRITICAL(); //Restores interrupt processes
      return;
  }else{
      EMU_EnterEM3(true);
      CORE_EXIT_CRITICAL(); //Restores interrupt processes
      return;
  }

  CORE_EXIT_CRITICAL(); //Restores interrupt processes
}

/***************************************************************************//**
 * @brief
 * This function returns the energy modes that are currently blocked
 *
 * @details
 * This function returns the index of the lowest energy modes array, which correlates to the energy mode that is disabled.
 *
 * @note
 * This function can be used to check which sleep mode operations are available and unavailable
 *
 * @return
 * Index of lowest energy mode, which correlates to the energy mode that is disabled.
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void){
  int i = 0;
  while(i < MAX_ENERGY_MODES){
      if(lowest_energy_modes[i] != 0){
          return i;
      }else{
          i++;
      }
  }
    return MAX_ENERGY_MODES - 1;
}



