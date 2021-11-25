/**
 * @file
 * letimer.c
 * @author
 * Adam Vitti
 * @date
 * 9/23/21
 * @brief
 * Module that creates and updates an event scheduler
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "scheduler.h"


//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int event_scheduled;



//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Initialize the event scheduler to 0.
 *
 *
 *
 * @details
 * Event scheduler will be initialized to 0, because no events are sheduled at start of program.
 *
 *
 * @note
 * This function will be called once in the app.c in order to initialize the scheduler. This should be initialized before interrupts are enabled.

 ******************************************************************************/
void scheduler_open(void){
  event_scheduled = 0;
}

/***************************************************************************//**
 * @brief
 * Adds events to the event scheduler.
 *
 *
 * @details
 * When adding events to the event scheduler, create an atomic event in order to disable interrupts
 * from disrupting this process.
 *
 *
 * @note
 * This function will be called when an event triggers an interrupt. The interrupt handler will call this function and
 * schedule the appropriate event.
 *
 *
 *
 * @param[in] event
 *  The "event" parameter is a uint32 that will containt a 1 bit in the position of the event we would like to schedule.
 *
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event){
  /* Atomic event */
  CORE_DECLARE_IRQ_STATE; //Save IRQ state
  CORE_ENTER_CRITICAL(); //disables interrupts and saves IEN bit

  event_scheduled |= event; //adds event to scheduler

  CORE_EXIT_CRITICAL(); //Restores interrupt processes
}

/***************************************************************************//**
 * @brief
 * This function removes events from the event scheduler.
 *
 *
 * @details
 *  When removing events from the event scheduler, create an atomic event in order to disable interrupts
 * from disrupting this process.
 *
 * @note
 * This function will be called after an event that was scheduled has been handled.
 *
 *
 * @param[in] event
 * The "event" parameter is the current event that has just been handled.
 *
 ******************************************************************************/
void remove_scheduled_event(uint32_t event){
  /* Atomic event */
  CORE_DECLARE_IRQ_STATE; //Save IRQ state
  CORE_ENTER_CRITICAL(); //disables interrupts and saves IEN bit

  event_scheduled &= ~event; //removes event from scheduler

  CORE_EXIT_CRITICAL(); //Restores interrupt processes
}

/***************************************************************************//**
 * @brief
 * Returns an integer value indicating which events are scheduled to be handled.
 *
 *
 * @details
 * This function can be used to check the status of an event and see whether it has been handled
 *
 *
 * @note
 * Will return a 0 if no events are scheduled.

 ******************************************************************************/
uint32_t get_scheduled_events(void){
  return event_scheduled;
}



