
// error.h

#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

// Define error codes
#define ERROR_INVALID_PARAM 0x01
#define ERROR_UNSUPPORTED_OPERATION 0x02
// #define ERROR_MEMORY_ALLOCATION_FAIL 0x02
// more error codes

//Report error
void report_error(uint32_t error_code);

#endif // ERROR_H


//#ifndef RISCV_CONSOLE_RISCV_FIRMWARE_SRC_ERROR_H_
//#define RISCV_CONSOLE_RISCV_FIRMWARE_SRC_ERROR_H_

//#endif //RISCV_CONSOLE_RISCV_FIRMWARE_SRC_ERROR_H_
