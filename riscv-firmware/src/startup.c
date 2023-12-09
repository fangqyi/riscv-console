#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include <stdbool.h>

extern uint8_t _erodata[];
extern uint8_t _data[];
extern uint8_t _edata[];
extern uint8_t _sdata[];
extern uint8_t _esdata[];
extern uint8_t _bss[];
extern uint8_t _ebss[];

// Adapted from https://stackoverflow.com/questions/58947716/how-to-interact-with-risc-v-csrs-by-using-gcc-c-code
// turn manipulations on risc-v Control and Status Registers (CSR) to callable functions with c

// Read the `mstatus` CSR (Machine Status Register).
//
// The `mstatus` register holds the machine's current operating state. It
// contains various control and status fields which include interrupt enable
// bits, privilege mode selection, etc.
__attribute__((always_inline)) inline uint32_t csr_mstatus_read(void)
{
    uint32_t result;
    asm volatile("csrr %0, mstatus" : "=r"(result));
    return result;
}

// Write to the `mstatus` CSR.
//
// This function is used to modify the machine's current operating state
// by updating the various control and status fields in the `mstatus` register.
__attribute__((always_inline)) inline void csr_mstatus_write(uint32_t val)
{
    asm volatile("csrw mstatus, %0" : : "r"(val));
}

// Write to the `mie` CSR (Machine Interrupt Enable Register).
//
// The `mie` register holds interrupt enable bits for the machine mode.
// Setting specific bits in this register enables corresponding interrupts.
__attribute__((always_inline)) inline void csr_write_mie(uint32_t val)
{
    asm volatile("csrw mie, %0" : : "r"(val));
}

// Enable machine mode interrupts.
//
// This function sets the `mie` bit in the `mstatus` register,
// which allows machine mode interrupts to be handled when they occur.
__attribute__((always_inline)) inline void csr_enable_interrupts(void)
{
    asm volatile("csrsi mstatus, 0x8");
}

// Disable machine mode interrupts.
//
// This function clears the `mie` bit in the `mstatus` register,
// preventing machine mode interrupts from being handled.
__attribute__((always_inline)) inline void csr_disable_interrupts(void)
{
    asm volatile("csrci mstatus, 0x8");
}

// chipset macros
// TODO: not done
#define INTERRUPT_ENABLE (*((volatile uint32_t *)0x40000000))  // interrupt enable register
#define INTERRUPT_PENDING (*((volatile uint32_t *)0x40000004)) // interrupt pending Register
#define MTIME_LOW (*((volatile uint32_t *)0x40000008))         // machine time
#define MTIME_HIGH (*((volatile uint32_t *)0x4000000C))
#define MTIMECMP_LOW (*((volatile uint32_t *)0x40000010)) // machine time compare
#define MTIMECMP_HIGH (*((volatile uint32_t *)0x40000014))
#define CONTROLLER (*((volatile uint32_t *)0x40000018)) // multi-button constroller status register
// video controller
#define MODE_CONTROL (*((volatile uint32_t *)0x500F6780)) // mode control register
#define TEXT_MODE 0x0
#define GRAPHICS_MODE 0x1

/*-----------------------------------mem maps of video controller-------------------------------*/
// mem map for bacground data 0x90000 (576KiB)
volatile uint32_t *BACKGROUND_DATA = (volatile uint32_t *)(0x50000000);

// mem map for large sprite data 0x40000 (256KiB)
volatile uint8_t *LARGE_SPRITE_DATA = (volatile uint8_t *)(0x50090000);

// mem map for memdium sprite data 0x10000 (64KiB)
volatile uint8_t *MEDIUM_SPRITE_DATA = (volatile uint8_t *)(0x500D0000);

// mem map for small sprite data 0x10000 (64KiB)
volatile uint8_t *SMALL_SPRITE_DATA = (volatile uint8_t *)(0x500E0000);

// mem map for background  0x1000 (4KiB)
volatile uint32_t *BACKGROUND_PALETTE = (volatile uint32_t *)(0x500F0000);

// mem map for large sprite palette 0x1000 (4KiB)
volatile uint32_t *LARGE_SPRITE_PALETTE = (volatile uint32_t *)(0x500F1000);

// mem map for medium sprite palette 0x1000 (4KiB)
volatile uint32_t *MEDIUM_SPRITE_PALETTE = (volatile uint32_t *)(0x500F2000);

// mem map for small sprite palette 0x1000 (4KiB)
volatile uint32_t *SMALL_SPRITE_PALETTE = (volatile uint32_t *)(0x500F3000);

// mem map for font data 0x800 (2KiB)
volatile uint32_t *FONT_DATA = (volatile uint32_t *)(0x500F4000);

// mem map for text data 0x900 (2.25KiB)
volatile char *TEXT_DATA = (volatile char *)(0x500F4800);

// mem map for text color 0x900 (2.25KiB)
volatile uint32_t *TEXT_COLOR = (volatile uint32_t *)(0x500F5100);

// mem map for background controls 0x100(256B)
volatile uint32_t *BACKGROUND_CONTROL = (volatile uint32_t *)(0x500F5A00);

// mem map for large sprite control 0x400 (1KiB)
volatile uint32_t *LARGE_SPRITE_CONTROL = (volatile uint32_t *)(0x500F5B00);

// mem map for medium sprite control 0x400 (1KiB)
volatile uint32_t *MEDIUM_SPRITE_CONTROL = (volatile uint32_t *)(0x500F5F00);

// mem map for small sprite control 0x400 (1KiB)
volatile uint32_t *SMALL_SPRITE_CONTROL = (volatile uint32_t *)(0x500F6300);

// mem map for small sprite palette 0x80 (128B)
volatile uint32_t *TEXT_PALETTE = (volatile uint32_t *)(0x500F6700);

extern volatile int global;
extern volatile uint32_t controller_status;
volatile uint32_t cmd_status;
volatile uint32_t interrupt_pending_reg;

/*--------------------------------------------------------------------------------------------*/
void simple_medium_sprite(uint16_t x, uint16_t y, uint16_t z);
void init(void)
{
    uint8_t *Source = _erodata;
    uint8_t *Base = _data < _sdata ? _data : _sdata;
    uint8_t *End = _edata > _esdata ? _edata : _esdata;

    while (Base < End)
    {
        *Base++ = *Source++;
    }
    Base = _bss;
    End = _ebss;
    while (Base < End)
    {
        *Base++ = 0;
    }

    // INTERRUPT_ENABLE = INTERRUPT_ENABLE | 0x1; // enable cartidge interrupts
    // INTERRUPT_ENABLE = INTERRUPT_ENABLE | 0x2; // enable video interrupts
    // INTERRUPT_ENABLE = INTERRUPT_ENABLE | 0x4; // enable command interrupts

    csr_write_mie(0x888);    // Enable all interrupt sources ^ is the above still necessary
    csr_enable_interrupts(); // Global interrupt enable
    MTIMECMP_LOW = 1;
    MTIMECMP_HIGH = 0;
    cmd_status = 0;
}

extern volatile int global;
extern volatile uint32_t controller_status;
volatile uint32_t interrupt_pending_reg;

// Store the most recent error code
volatile uint32_t last_error_code = 0;

// Set the last error code
void set_last_error_code(uint32_t error_code)
{
    last_error_code = error_code;
}
// Get the last error code
uint32_t get_last_error_code()
{
    return last_error_code;
}

void c_interrupt_handler(void)
{
    uint64_t new_compare = (((uint64_t)MTIMECMP_HIGH) << 32) | MTIMECMP_LOW;
    new_compare += 100;
    MTIMECMP_HIGH = new_compare >> 32;
    MTIMECMP_LOW = new_compare;
    global++;
    controller_status = CONTROLLER;
    interrupt_pending_reg = INTERRUPT_PENDING;
    if ((INTERRUPT_PENDING & 0x4) == 0)
    {
        cmd_status = 1;
    }
    INTERRUPT_PENDING = INTERRUPT_PENDING | 0x4; // clear command interrupts
    INTERRUPT_PENDING = INTERRUPT_PENDING | 0x2; // clear video interrupts
    INTERRUPT_PENDING = INTERRUPT_PENDING | 0x1; // clear cartidge interrupts
}

uint32_t get_interrupt_pending_reg()
{
    return interrupt_pending_reg;
}

uint32_t get_cmd_status()
{
    uint32_t ret = cmd_status;
    cmd_status = 0; // clear cmd_status
    return ret;
}

uint32_t get_vid_pending_bit()
{
    return (get_interrupt_pending_reg() & 0x1);
}

// key_idx: 0 LEFT, 1 UP, 2 DOWN, 3 RIGHT
uint32_t get_controller_status_key(uint8_t key_idx)
{
    return controller_status & (1 << key_idx);
}

/*
uint64_t get_machine_time(){
    return (((uint64_t)MTIME_HIGH)<<32) | MTIME_LOW;
}*/

uint32_t get_machine_time()
{
    return MTIME_LOW;
}

uint32_t get_controller_status()
{
    return CONTROLLER; // controller_status;
}

uint32_t RED = 0xFFFF0000;
uint32_t BLUE = 0xFF0000FF;
uint32_t GREEN = 0xFF00FF00;
uint32_t WHITE = 0xFFFFFFFF;

void my_memcpy(uint8_t *dst, uint8_t *src, size_t num)
{
    for (size_t i = 0; i < num; i++)
    {
        dst[i] = src[i];
    }
}

// controls for background
void set_pixel_background_data(uint8_t image_idx, char *data)
{
    char *dst = ((char *)BACKGROUND_DATA) + image_idx * 0x24000;
    my_memcpy(dst, data, 0x24000);
}

void set_tile_background_data(uint8_t image_idx, char *data)
{
    char *dst = ((char *)BACKGROUND_DATA) + image_idx * 0x12000;
    my_memcpy(dst, data, 0x12000);
}

void set_pixel_background_control(
    uint8_t control_idx,
    uint8_t px_idx,
    uint16_t x,
    uint16_t y,
    uint8_t z,
    uint8_t palette_idx)
{
    BACKGROUND_CONTROL[control_idx] = (px_idx << 29) | (z << 22) | ((y + 288) << 12) | ((x + 512) << 2) | palette_idx;
}

/* changed name to be set_tile_background_control from set_text_background_control*/

void set_tile_background_control(
    uint8_t control_idx,
    uint8_t tile_idx,
    uint8_t sub_idx,
    uint16_t x,
    uint16_t y,
    uint8_t z,
    uint8_t palette_idx)
{
    BACKGROUND_CONTROL[control_idx] = (1 << 31) | (tile_idx << 28) | (sub_idx << 25) | (z << 22) | ((y + 288) << 12) | ((x + 512) << 2) | palette_idx;
}

void set_background_palette(uint8_t palette_idx, uint32_t *data)
{
    char *dst = ((char *)BACKGROUND_PALETTE) + palette_idx * 0x400;
    my_memcpy(dst, data, 0x400);
}

// controls for small sprites
void set_small_sprite_data(uint8_t sprite_idx, uint8_t *data)
{
    char *dst = ((char *)SMALL_SPRITE_DATA) + sprite_idx * 0x100;
    my_memcpy(dst, data, 0x100);
}

void set_small_sprite_control(
    uint8_t sprite_ctrl_idx,
    uint8_t sprite_data_idx,
    uint16_t x,
    uint16_t y,
    uint16_t z,
    uint8_t palette_idx)
{
    SMALL_SPRITE_CONTROL[sprite_ctrl_idx] = (sprite_data_idx << 24) | (z << 21) | ((y + 16) << 12) | ((x + 16) << 2) | palette_idx;
}

void set_small_sprite_palette(uint8_t palette_idx, uint32_t *data)
{
    char *dst = ((char *)SMALL_SPRITE_PALETTE) + palette_idx * 0x400;
    my_memcpy(dst, data, 0x400);
}

// controls for medium sprites
void set_medium_sprite_data(uint8_t sprite_idx, uint8_t *data)
{
    char *dst = ((char *)MEDIUM_SPRITE_DATA) + sprite_idx * 0x400;
    my_memcpy(dst, data, 0x400);
}

void set_medium_sprite_control(
    uint8_t sprite_ctrl_idx,
    uint8_t sprite_data_idx,
    uint16_t x,
    uint16_t y,
    uint16_t z,
    uint8_t palette_idx)
{
    MEDIUM_SPRITE_CONTROL[sprite_ctrl_idx] = (sprite_data_idx << 24) | (z << 21) | ((y + 32) << 12) | ((x + 32) << 2) | palette_idx;
}

void set_medium_sprite_palette(uint8_t palette_idx, uint32_t *data)
{
    char *dst = ((char *)MEDIUM_SPRITE_PALETTE) + palette_idx * 0x400;
    my_memcpy(dst, data, 0x400);
}

// // controls for large sprites
void set_large_sprite_data(uint8_t sprite_idx, uint8_t *data)
{
    char *dst = ((char *)LARGE_SPRITE_DATA) + sprite_idx * 0x1000;
    my_memcpy(dst, data, 0x1000);
}

void set_large_sprite_control(
    uint8_t sprite_ctrl_idx,
    uint8_t sprite_data_idx,
    uint16_t x,
    uint16_t y,
    uint16_t z,
    uint8_t palette_idx)
{
    LARGE_SPRITE_CONTROL[sprite_ctrl_idx] = (sprite_data_idx << 24) | (z << 21) | ((y + 64) << 12) | ((x + 64) << 2) | palette_idx;
}

void set_large_sprite_palette(uint8_t palette_idx, uint32_t *data)
{
    char *dst = ((char *)LARGE_SPRITE_PALETTE) + palette_idx * 0x400;
    my_memcpy(dst, data, 0x400);
}

/* ADDED new functions below */
/* the type of control_ind is int or unit8_t */
/* FIXME: should be either pixel or tile mode*/

void setPixelBackground(uint8_t image_idx, char *bg_pixel_data, uint8_t control_idx, uint8_t px_idx, uint16_t x, uint16_t y, uint8_t z, uint8_t palette_idx, uint32_t *palette_data)
{
    set_pixel_background_data(image_idx, bg_pixel_data);
    set_pixel_background_control(control_idx, px_idx, x, y, z, palette_idx);
    set_background_palette(palette_idx, palette_data);
}

void setTileBackground(uint8_t image_idx, char *bg_tile_data, uint8_t control_idx, uint8_t tile_idx, uint8_t sub_idx, uint16_t x, uint16_t y, uint8_t z, uint8_t palette_idx, uint32_t *palette_data)
{
    set_tile_background_data(image_idx, bg_tile_data);
    set_tile_background_control(control_idx, tile_idx, sub_idx, x, y, z, palette_idx);
    set_background_palette(palette_idx, palette_data);
}

void setLargeSprite(uint8_t *data, uint8_t sprite_ctrl_idx, uint8_t sprite_data_idx, uint16_t x, uint16_t y, uint16_t z, uint8_t palette_idx, uint32_t *palette_data)
{
    MODE_CONTROL = GRAPHICS_MODE;
    set_large_sprite_control(sprite_ctrl_idx, sprite_data_idx, x, y, z, palette_idx);
    set_large_sprite_data(sprite_data_idx, data);
    set_large_sprite_palette(palette_idx, palette_data);
}

void setMediumSprite(uint8_t *data, uint8_t sprite_ctrl_idx, uint8_t sprite_data_idx, uint16_t x, uint16_t y, uint16_t z, uint8_t palette_idx, uint32_t *palette_data)
{
    MODE_CONTROL = GRAPHICS_MODE;
    set_medium_sprite_control(sprite_ctrl_idx, sprite_data_idx, x, y, z, palette_idx);
    set_medium_sprite_data(sprite_data_idx, data);
    set_medium_sprite_palette(palette_idx, palette_data);
}

void setSmallSprite(uint8_t *data, uint8_t sprite_ctrl_idx, uint8_t sprite_data_idx, uint16_t x, uint16_t y, uint16_t z, uint8_t palette_idx, uint32_t *palette_data)
{
    MODE_CONTROL = GRAPHICS_MODE;
    set_small_sprite_control(sprite_ctrl_idx, sprite_data_idx, x, y, z, palette_idx);
    set_small_sprite_data(sprite_data_idx, data);
    set_small_sprite_palette(palette_idx, palette_data);
}

void switch_mode(uint32_t mode)
{
    MODE_CONTROL = (mode == 0) ? TEXT_MODE : GRAPHICS_MODE;
}

void simple_display_text(char *new_text, uint32_t start_idx)
{
    // uint32_t bounds  = 0x900>>2;
    MODE_CONTROL = TEXT_MODE;
    uint32_t offset = 0;
    while (new_text[offset] != '\0')
    { //&& start_idx + offset < bounds){
        TEXT_DATA[start_idx + offset] = new_text[offset++];
    }
}

void simple_medium_sprite(uint16_t x, uint16_t y, uint16_t z)
{
    MODE_CONTROL = GRAPHICS_MODE;

    uint8_t sprite_data[0x400];
    uint32_t palette_data[0x100];
    for (int i = 0; i < 0x20; i++)
    {
        for (int j = 0; j < 0x20; j++)
        {
            palette_data[(i * 0x20 + j) % 0x100] = 0;
            sprite_data[i * 0x20 + j] = i < 0x10 ? 0 : 1;
        }
    }
    palette_data[0] = GREEN;
    palette_data[1] = RED;

    // char buf[64];
    // snprintf(buf, sizeof(buf), "%d, %d, %d", x, y, z);
    // simple_display_text(buf, 0);
    set_medium_sprite_palette(2, palette_data);
    set_medium_sprite_data(10, sprite_data);
    set_medium_sprite_control(5, 10, x, y, z, 2);
}

typedef void (*TThreadEntry)(void *);
typedef uint32_t *TThreadContext;

void OtherThreadFunction(void *);

// this can be thought as fork() function
TThreadContext InitThread(uint32_t *stacktop, TThreadEntry entry, void *param);

void SwitchThread(TThreadContext *oldcontext, TThreadContext newcontext);

TThreadContext OtherThread;
TThreadContext MainThread;

void OtherThreadFunction(void *)
{
    int last_global = global;
    while (1)
    {
        if (global != last_global)
        {
            SwitchThread(&OtherThread, MainThread);
            last_global = global;
        }
    }
}

typedef struct
{
    volatile bool locked;
} my_mutex_t;

void my_mutex_init(my_mutex_t *mutex)
{
    mutex->locked = false;
}

void my_mutex_lock(my_mutex_t *mutex)
{
    while (true)
    {

        if (!mutex->locked)
        {
            mutex->locked = true;
            break; // Successfully acquired the mutex
        }
    }
}

void my_mutex_unlock(my_mutex_t *mutex)
{

    mutex->locked = false;
}

/* -------- Syscall -------- */

// Define constants for system call operations
enum SysCallOperation
{
    TEST = 0,
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
    ERROR_HANDLER_OPERATION = 11, // Add ERROR_HANDLER_OPERATION
    INIT_THREAD = 12,
    SWITCH_THREAD = 13,
    GET_CMD_STATUS = 14,
    GET_VID_PENDING = 15,
    GET_CONTROLLER_KEY_STATUS = 16,
    GET_GLOBAL_TIME = 17,
    INIT_MUTEX = 18,
    MUTEX_LOCK = 19,
    MUTEX_UNLOCK = 20,
    SET_TILE_BACKGROUND = 21,
};

uint32_t c_syscall(uint32_t *param, char *params)
{

    if (param == NULL)
    {
        // Handle invalid input
        // return -1;  // Or an appropriate error code
        set_last_error_code(ERROR_NULL_PARAMS);
        return -1; // Null params error
    }

    switch (param[0])
    {
    case TEST:
        simple_medium_sprite((uint16_t)param[1], (uint16_t)param[2], (uint16_t)param[3]);
        return 0;

    case GET_TIMER_TICKS:
        return get_machine_time();

    case GET_GLOBAL_TIME:
        return global;

    case GET_MODE_CONTROL_REGISTER:
        return MODE_CONTROL;

    case GET_CONTROLLER_REGISTER:
        return get_controller_status();

    case GET_INTERRUPT_PENDING_REGISTER:
        return interrupt_pending_reg;

    case SWITCH_MODE:
        if (param[1] == 0 || param[1] == 1)
        {
            switch_mode((uint32_t)param[1]);
            return 0; // Success
        }
        else
        {
            // Handle invalid mode value
            // return -1;  // Or an appropriate error code
            set_last_error_code(ERROR_INVALID_MODE_VALUE);
            return -1; // Invalid mode value error
        }

    case SET_BACKGROUND:
        setPixelBackground((uint8_t)param[1], (char *)param[2], (uint8_t)param[3], (uint8_t)param[4], (uint16_t)param[5], (uint16_t)param[6], (uint8_t)param[7], (uint8_t)param[8], (uint32_t *)param[9]);
        return 0;

    case SET_TILE_BACKGROUND:
        setTileBackground((uint8_t)param[1], (char *)param[2], (uint8_t)param[3], (uint8_t)param[4], (uint8_t)param[5], (uint16_t)param[6], (uint16_t)param[7], (uint8_t)param[8], (uint8_t)param[9], (uint32_t *)param[10]);
        return 0;

    case DISPLAY_TEXT:
        if (params != NULL)
        {
            simple_display_text((char *)params, (uint32_t)param[1]);
            return 0; // Success
        }
        else
        {
            // Handle invalid params pointer
            //  return -1;  // Or an appropriate error code
            set_last_error_code(ERROR_NULL_PARAMS);
            return -1; // Null params error
        }

    case SET_SMALL_SPRITE:
        // Validate parameters and call setSmallSprite
        setSmallSprite((uint8_t *)param[1], (uint8_t)param[2], (uint8_t)param[3], (uint16_t)param[4], (uint16_t)param[5], (uint16_t)param[6], (uint8_t)param[7], (uint32_t *)param[8]);
        return 0; // Success

    case SET_MEDIUM_SPRITE:
        // Validate parameters and call setMediumSprite
        setMediumSprite((uint8_t *)param[1], (uint8_t)param[2], (uint8_t)param[3], (uint16_t)param[4], (uint16_t)param[5], (uint16_t)param[6], (uint8_t)param[7], (uint32_t *)param[8]);
        return 0; // Success

    case SET_LARGE_SPRITE:
        // Validate parameters and call setLargeSprite
        setLargeSprite((uint8_t *)param[1], (uint8_t)param[2], (uint8_t)param[3], (uint16_t)param[4], (uint16_t)param[5], (uint16_t)param[6], (uint8_t)param[7], (uint32_t *)param[8]);
        return 0; // Success

    case ERROR_HANDLER_OPERATION:
        // Return the last error code and reset it
        uint32_t error = get_last_error_code();
        set_last_error_code(0); // Reset the error code after retrieval
        return error;

    case INIT_THREAD:
        // (uint32_t *stacktop, ThreadEntry entry, void *param
        InitThread((uint32_t *)param[1], (TThreadEntry)param[2], (void *)param[3]);
        return 0;

    case SWITCH_THREAD:
        // ThreadContext *oldcontext, ThreadContext newcontext
        SwitchThread((TThreadContext *)param[1], (TThreadContext)param[2]);
        return 0;

    case GET_CMD_STATUS:
        return get_cmd_status();

    case GET_VID_PENDING:
        return get_vid_pending_bit();

    case GET_CONTROLLER_KEY_STATUS:
        uint8_t key_idx = (uint8_t)param[1];
        if ((0 <= key_idx) && (key_idx <= 3))
            return get_controller_status_key(key_idx);
        else
            set_last_error_code(ERROR_INVALID_INDEX);
        return -1; // invalid key index error

    case INIT_MUTEX:
        my_mutex_init((my_mutex_t *)param[1]);
        return 0;

    case MUTEX_LOCK:
        my_mutex_lock((my_mutex_t *)param[1]);
        return 0;

    case MUTEX_UNLOCK:
        my_mutex_unlock((my_mutex_t *)param[1]);
        return 0;

    default:
        // Handle unknown operation
        // error handler
        set_last_error_code(ERROR_UNSUPPORTED_OPERATION);
        return ERROR_UNSUPPORTED_OPERATION;
        // return -1;  // Or an appropriate error code
    }
}
