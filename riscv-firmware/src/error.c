// error.c

#include "error.h"
#include <stdio.h>

void report_error(uint32_t error_code) {
  // Print error
  printf("Error occurred: %u\n", error_code);

  // Error handling based on the error code
  switch (error_code) {
    case ERROR_INVALID_PARAM:
      // Handle invalid parameter error
      printf("Invalid parameter provided.\n");
      break;

    case ERROR_UNSUPPORTED_OPERATION:
      // Handle unsupported operation
      printf("Unsupported operation.\n");
      break;

      // Other error codes

    default:
      printf("Unknown error.\n");
      break;
  }

  // Attempt handle error
  // Restart the system

}

