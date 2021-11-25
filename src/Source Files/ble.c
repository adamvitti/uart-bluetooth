/**
 * @file ble.c
 * @author Adam Vitti
 * @date 11/7/21
 * @brief Contains all the functions to interface the application with the HM-18
 *   BLE module and the LEUART driver
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "ble.h"
#include <string.h>

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************


/***************************************************************************//**
 * @brief BLE module
 * @details
 *  This module contains all the functions to interface the application layer
 *  with the HM-18 Bluetooth module.  The application does not have the
 *  responsibility of knowing the physical resources required, how to
 *  configure, or interface to the Bluetooth resource including the LEUART
 *  driver that communicates with the HM-18 BLE module.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *  Function to initialize leuart peripheral for use with the HM10 ble module
 *
 * @details
 *  This function constructs an open struct for the LEUART0 peripheral with settings/values specific to the bluetooth peripheral.
 *  The function then initializes the leuart peripheral with these specific values and set up for bluetooth operation.
 *
 * @param[in] tx_event
 *  Callback event to be serviced upon completion of a bluetooth transmit operation
 *
 * @param[in] rx_event
 *  Callback event to be serviced upon completion of a bluetooth receive operation
 ******************************************************************************/

void ble_open(uint32_t tx_event, uint32_t rx_event){
  LEUART_OPEN_STRUCT ble_leuart_open_struct;

    //timer_delay(25); // 25ms for startup of sensor

    ble_leuart_open_struct.baudrate = HM10_BAUDRATE ;
    ble_leuart_open_struct.databits = HM10_DATABITS  ;
    ble_leuart_open_struct.enable =  HM10_ENABLE ;
    ble_leuart_open_struct.parity = HM10_PARITY ;
    ble_leuart_open_struct.refFreq = HM10_REFFREQ ;
    ble_leuart_open_struct.rx_done_evt = rx_event ;
    ble_leuart_open_struct.rx_en = LEUART_DEFAULT  ;
    ble_leuart_open_struct.rx_loc = LEUART0_RX_ROUTE ;
    ble_leuart_open_struct.rx_pin_en = LEUART_DEFAULT ;
//    ble_leuart_open_struct.rxblocken = ;
//    ble_leuart_open_struct.sfubrx = ;
//    ble_leuart_open_struct.sigframe = ;
//    ble_leuart_open_struct.sigframe_en = ;
//    ble_leuart_open_struct.startframe = ;
    ble_leuart_open_struct.stopbits = HM10_STOPBITS;
    ble_leuart_open_struct.tx_done_evt = tx_event;
    ble_leuart_open_struct.tx_en = LEUART_DEFAULT ;
    ble_leuart_open_struct.tx_loc = LEUART0_TX_ROUTE ;
    ble_leuart_open_struct.tx_pin_en = LEUART_DEFAULT ;
    ble_leuart_open_struct.txc_irq_enable = LEUART_DEFAULT ;
    ble_leuart_open_struct.txbl_irq_enable = LEUART_DEFAULT ;


    leuart_open(HM10_LEUART0, &ble_leuart_open_struct);

}


/***************************************************************************//**
 * @brief
 *  Transmits a string of data across a bluetooth connection
 *
 * @details
 * This function will send a string of data to the leuart peripheral, therefore, sending it through the bluetooth module.
 * To transmit the data, this function calls leuart_start with the string of data and length of data to be sent.
 *
 * @param[in] *string
 *  Data to be sent across bluetooth connection
 *
 ******************************************************************************/

void ble_write(char* string){
  size_t length = strlen(string);
  leuart_start(LEUART0, string, length);
}

/***************************************************************************//**
 * @brief
 *   BLE Test performs two functions.  First, it is a Test Driven Development
 *   routine to verify that the LEUART is correctly configured to communicate
 *   with the BLE HM-18 module.  Second, the input argument passed to this
 *   function will be written into the BLE module and become the new name
 *   advertised by the module while it is looking to pair.
 *
 * @details
 *   This global function will use polling functions provided by the LEUART
 *   driver for both transmit and receive to validate communications with
 *   the HM-18 BLE module.  For the assignment, the communication with the
 *   BLE module must use low energy design principles of being an interrupt
 *   driven state machine.
 *
 * @note
 *   For this test to run to completion, the phone most not be paired with
 *   the BLE module.  In addition for the name to be stored into the module
 *   a breakpoint must be placed at the end of the test routine and stopped
 *   at this breakpoint while in the debugger for a minimum of 5 seconds.
 *
 * @param[in] *mod_name
 *   The name that will be written to the HM-18 BLE module to identify it
 *   while it is advertising over Bluetooth Low Energy.
 *
 * @return
 *   Returns bool true if successfully passed through the tests in this
 *   function.
 ******************************************************************************/

bool ble_test(char *mod_name){
  int str_len;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // This test will limit the test to the proper setup of the LEUART
  // peripheral, routing of the signals to the proper pins, pin
  // configuration, and transmit/reception verification.  The test
  // will communicate with the BLE module using polling routines
  // instead of interrupts.
  // How is polling different than using interrupts?
  // ANSWER: In polling, cpu continuously checks if device/operation needs to be handled, whereas
  //         in interrupts, the device/operation notifies the cpu that it needs to be handled
  // How does interrupts benefit the system for low energy operation?
  // ANSWER: The CPU can be put in lower energy modes, and will not be continuously running while no interrupts occur
  // How does interrupts benefit the system that has multiple tasks?
  // ANSWER: We can set priorities to tasks with interrupts, whereas, polling will handle on a first come first serve basis

  // First, you will need to review the DSD HM10 datasheet to determine
  // what the default strings to write data to the BLE module and the
  // expected return statement from the BLE module to test / verify the
  // correct response

  // The test_str is used to tell the BLE module to end a Bluetooth connection
  // such as with your phone.  The ok_str is the result sent from the BLE module
  // to the micro-controller if there was not active BLE connection at the time
  // the break command was sent to the BLE module.
  // Replace the test_str "" with the command to break or end a BLE connection
  // Replace the ok_str "" with the result that will be returned from the BLE
  //   module if there was no BLE connection
  char    test_str[80] = "AT";
  char    ok_str[80] = "OK";


  // output_str will be the string that will program a name to the BLE module.
  // From the DSD HM10 datasheet, what is the command to program a name into
  // the BLE module?
  // The  output_str will be a string concatenation of the DSD HM10 command
  // and the input argument sent to the ble_test() function
  // Replace the output_str "" with the command to change the program name
  // Replace the result_str "" with the first part of the expected result
  // The HM-10 datasheet has an error. This response starts with "OK+Set:"
  //  the backend of the expected response will be concatenated with the
  //  input argument
  char    output_str[80] = "AT+NAME";
  char    result_str[80] = "OK+Set:";


  // To program the name into your module, you must reset the module after you
  // have sent the command to update the modules name.  What is the DSD HM10
  // name to reset the module?
  // Replace the reset_str "" with the command to reset the module
  // Replace the reset_result_str "" with the expected BLE module response to
  //  to the reset command
  char    reset_str[80] = "AT+RESET";
  char    reset_result_str[80] = "OK+RESET";
  char    return_str[80];

  bool    success;
  bool    rx_disabled, rx_en, tx_en;
  uint32_t  status;

  // These are the routines that will build up the entire command and response
  // of programming the name into the BLE module.  Concatenating the command or
  // response with the input argument name
  strcat(output_str, mod_name);
  strcat(result_str, mod_name);

  // The test routine must not alter the function of the configuration of the
  // LEUART driver, but requires certain functionality to insure the logical test
  // of writing and reading to the DSD HM10 module.  The following c-lines of code
  // save the current state of the LEUART driver that will be used later to
  // re-instate the LEUART configuration

  status = leuart_status(HM10_LEUART0);
  if (status & LEUART_STATUS_RXBLOCK) {
    rx_disabled = true;
    // Enabling, unblocking, the receiving of data from the LEUART RX port
    leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKDIS);
  }
  else rx_disabled = false;
  if (status & LEUART_STATUS_RXENS) {
    rx_en = true;
  } else {
    rx_en = false;
    // Enabling the receiving of data from the RX port
    leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXEN);
  // Why could you be stuck in the below while loop after a write to CMD register?
  // Answer: Getting stuck would indicate that the receiver was not enabled
    while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_RXENS));
  }

  if (status & LEUART_STATUS_TXENS){
    tx_en = true;
  } else {
    // Enabling the transmission of data to the TX port
    leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXEN);
    while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_TXENS));
    tx_en = false;
  }
//  leuart_cmd_write(HM10_LEUART0, (LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX));

  // This sequence of instructions is sending the break ble connection
  // to the DSD HM10 module.
  // Why is this command required if you want to change the name of the
  // DSD HM10 module?
  // ANSWER: Because we have to reset the HM10 module in order for the name to change
  str_len = strlen(test_str);
  for (int i = 0; i < str_len; i++){
    leuart_app_transmit_byte(HM10_LEUART0, test_str[i]);
  }

  // What will the ble module response back to this command if there is
  // a current ble connection?
  // ANSWER: Response = "OK+LOST"
  str_len = strlen(ok_str);
  for (int i = 0; i < str_len; i++){
    return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
    if (ok_str[i] != return_str[i]) {
        EFM_ASSERT(false);;
    }
  }

  // This sequence of code will be writing or programming the name of
  // the module to the DSD HM10
  str_len = strlen(output_str);
  for (int i = 0; i < str_len; i++){
    leuart_app_transmit_byte(HM10_LEUART0, output_str[i]);
  }

  // Here will be the check on the response back from the DSD HM10 on the
  // programming of its name
  str_len = strlen(result_str);
  for (int i = 0; i < str_len; i++){
    return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
    if (result_str[i] != return_str[i]) {
        EFM_ASSERT(false);;
    }
  }

  // It is now time to send the command to RESET the DSD HM10 module
  str_len = strlen(reset_str);
  for (int i = 0; i < str_len; i++){
    leuart_app_transmit_byte(HM10_LEUART0, reset_str[i]);
  }

  // After sending the command to RESET, the DSD HM10 will send a response
  // back to the micro-controller
  str_len = strlen(reset_result_str);
  for (int i = 0; i < str_len; i++){
    return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
    if (reset_result_str[i] != return_str[i]) {
        EFM_ASSERT(false);;
    }
  }

  // After the test and programming have been completed, the original
  // state of the LEUART must be restored
  if (!rx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXDIS);
  if (rx_disabled) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKEN);
  if (!tx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXDIS);
  leuart_if_reset(HM10_LEUART0);

  success = true;


  CORE_EXIT_CRITICAL();
  return success;
}



