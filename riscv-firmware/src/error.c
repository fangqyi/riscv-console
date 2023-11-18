// error.c

#include "error.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
  uint64_t syscall_params_error[2];
  syscall_params_error[0] = DISPLAY_TEXT;
  c_syscall(syscall_params_error, errorMsg);

  // Attempt handle error
  // Restart the system

}

