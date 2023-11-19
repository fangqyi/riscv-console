// error.c

#include "error.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t SystemCall(uint64_t* param);
uint32_t SystemCall2(uint64_t* param1, char* param2);

enum SysCallOperation
{
  GET_TIMER_TICKS = 1,
  GET_MODE_CONTROL_REGISTER = 2,
  GET_CONTROLLER_REGISTER = 3,
  GET_INTERRUPT_PENDING_REGISTER = 4,
  SWITCH_MODE = 5,
  SET_BACKGROUND = 6,
  DISPLAY_TEXT = 7,
  SET_SMALL_SPRITE = 8,
  SET_MEDIUM_SPRITE = 9,
  SET_LARGE_SPRITE = 10,
};

void report_error(uint32_t error_code) {
  char errorMsg[100];

  // Error handling based on the error code
  switch (error_code) {
    case ERROR_INVALID_PARAM:
      snprintf(errorMsg, sizeof(errorMsg), "Error %u: Invalid parameter provided.\n", error_code);
      break;
    case ERROR_UNSUPPORTED_OPERATION:
      snprintf(errorMsg, sizeof(errorMsg), "Error %u: Unsupported operation.\n", error_code);
      break;

      // Other error cases
    default:
      snprintf(errorMsg, sizeof(errorMsg), "Error %u: Unknown error.\n", error_code);
      break;
  }

  // Display the error message using a system call
  uint64_t error_display_params[] = {DISPLAY_TEXT, 160};
  SystemCall2(error_display_params, errorMsg);

  // Attempt handle error
  // Restart the system

}

