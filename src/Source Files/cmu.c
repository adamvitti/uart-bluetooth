/**
 * @file
 * cmu.c
 * @author
 * Adam Vitti
 * @date
 * 9/23/21
 * @brief
 * Module that enables oscillators and routes clock tree
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Enables the high frequency clock and disables the low frequency oscillators. Then routes the clock tree.
 *
 * @details
 * This is a low level module that directly interfaces with the hardware, selecting the clock we want to use and routing it
 * to the correct location. This module sets up an ultra low frequency clock and connects it to the low frequency clock tree.
 *
 * @note
 * This function is generally called once to initialize our clock for ultra low frequency use.
 *
 ******************************************************************************/

void cmu_open(void){

    CMU_ClockEnable(cmuClock_HFPER, true);

    // By default, LFRCO is enabled, disable the LFRCO oscillator
    // Disable the LFRCO oscillator (Low frequency RC oscillator)
    // What is the enumeration required for LFRCO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);

    // Disable the LFXO oscillator (Low frequency crystal oscillator)
    // What is the enumeration required for LFXO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

    // No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H1

    // Route LF clock to the LF clock tree
    // What is the enumeration required to placed the ULFRCO onto the proper clock branch?
    // It can be found in the online HAL documentation
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);    // routing ULFRCO to proper Low Freq clock tree

    // What is the proper enumeration to enable the clock tree onto the LE clock branches?
    // It can be found in the Assignment 2 documentation
    CMU_ClockEnable(cmuClock_CORELE, true);



}


