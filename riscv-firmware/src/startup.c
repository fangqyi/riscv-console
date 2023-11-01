#include <stdint.h>

extern uint8_t _erodata[];
extern uint8_t _data[];
extern uint8_t _edata[];
extern uint8_t _sdata[];
extern uint8_t _esdata[];
extern uint8_t _bss[];
extern uint8_t _ebss[];
    

// Adapted from https://stackoverflow.com/questions/58947716/how-to-interact-with-risc-v-csrs-by-using-gcc-c-code
//turn manipulations on risc-v Control and Status Registers (CSR) to callable functions with c


// Read the `mstatus` CSR (Machine Status Register).
//
// The `mstatus` register holds the machine's current operating state. It
// contains various control and status fields which include interrupt enable 
// bits, privilege mode selection, etc.
__attribute__((always_inline)) inline uint32_t csr_mstatus_read(void){
    uint32_t result;
    asm volatile ("csrr %0, mstatus" : "=r"(result));
    return result;
}

// Write to the `mstatus` CSR.
//
// This function is used to modify the machine's current operating state 
// by updating the various control and status fields in the `mstatus` register.
__attribute__((always_inline)) inline void csr_mstatus_write(uint32_t val){
    asm volatile ("csrw mstatus, %0" : : "r"(val));
}

// Write to the `mie` CSR (Machine Interrupt Enable Register).
//
// The `mie` register holds interrupt enable bits for the machine mode. 
// Setting specific bits in this register enables corresponding interrupts.
__attribute__((always_inline)) inline void csr_write_mie(uint32_t val){
    asm volatile ("csrw mie, %0" : : "r"(val));
}

// Enable machine mode interrupts.
//
// This function sets the `mie` bit in the `mstatus` register, 
// which allows machine mode interrupts to be handled when they occur.
__attribute__((always_inline)) inline void csr_enable_interrupts(void){
    asm volatile ("csrsi mstatus, 0x8");
}

// Disable machine mode interrupts.
//
// This function clears the `mie` bit in the `mstatus` register, 
// preventing machine mode interrupts from being handled.
__attribute__((always_inline)) inline void csr_disable_interrupts(void){
    asm volatile ("csrci mstatus, 0x8");
}

//chipset macros
//TODO: not done
#define MTIME_LOW       (*((volatile uint32_t *)0x40000008)) // machine time 
#define MTIME_HIGH      (*((volatile uint32_t *)0x4000000C)) 
#define MTIMECMP_LOW    (*((volatile uint32_t *)0x40000010)) // machine time compare
#define MTIMECMP_HIGH   (*((volatile uint32_t *)0x40000014))
#define CONTROLLER      (*((volatile uint32_t *)0x40000018)) // multi-button constroller status register
//video controller
#define MODE_CONTROL    (*((volatile uint32_t *)0x500F6780)) // mode control register
#define TEXT_MODE       0x0
#define GRAPHICS_MODE   0x1

/*-----------------------------------mem maps of video controller-------------------------------*/     
// mem map for bacground data 0x90000 (576KiB)
volatile uint32_t *BACKGROUND_DATA= (volatile uint32_t *)(0x50000000); 

// mem map for large sprite data 0x40000 (256KiB)
volatile uint32_t *LARGE_SPRITE_DATA= (volatile uint32_t *)(0x50090000); 

// mem map for memdium sprite data 0x10000 (64KiB)
volatile uint32_t *MEDIUM_SPRITE_DATA= (volatile uint32_t *)(0x500D0000); 

// mem map for small sprite data 0x10000 (64KiB)
volatile uint32_t *SMALL_SPRITE_DATA= (volatile uint32_t *)(0x500E0000); 

// mem map for background  0x1000 (4KiB)
volatile uint32_t *BACKGROUND_PALETTE= (volatile uint32_t *)(0x500F0000); 

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

/*--------------------------------------------------------------------------------------------*/     

void init(void){
    uint8_t *Source = _erodata;
    uint8_t *Base = _data < _sdata ? _data : _sdata;
    uint8_t *End = _edata > _esdata ? _edata : _esdata;

    while(Base < End){
        *Base++ = *Source++;
    }
    Base = _bss;
    End = _ebss;
    while(Base < End){
        *Base++ = 0;
    }

    csr_write_mie(0x888);       // Enable all interrupt soruces
    csr_enable_interrupts();    // Global interrupt enable
    MTIMECMP_LOW = 1;
    MTIMECMP_HIGH = 0;
}

extern volatile int global;
extern volatile uint32_t controller_status;

void c_interrupt_handler(void){
    uint64_t new_compare = (((uint64_t)MTIMECMP_HIGH)<<32) | MTIMECMP_LOW;
    new_compare += 100;
    MTIMECMP_HIGH = new_compare>>32;
    MTIMECMP_LOW = new_compare;
    global++;
    controller_status = CONTROLLER;
}

uint32_t get_controller_status() {
    return controller_status;
}

// controls for background
void set_pixel_background_data(uint8_t image_idx, char* data) {
    char* dst = ((char*)BACKGROUND_DATA)[image*0x24000];
    memcpy(dst, data, 0x24000);
}

// TODO: impl setting background data for tiles

void set_pixel_background_control(
    uint8_t control_idx,
    uint8_t px_idx,
    uint16_t x,
    uint16_t y,
    uint8_t z, 
    uint8_t palette_idx
){
    BACKGROUND_CONTROL[control_idx] = (px_idx << 29) | (z << 22) | (y << 12) | (x << 2) | palette_idx;
}

void set_text_background_control(
    uint8_t control_idx,
    uint8_t tile_idx,
    uint8_t sub_idx,
    uint16_t x,
    uint16_t y,
    uint8_t z, 
    uint8_t palette_idx
){
    BACKGROUND_CONTROL[control_idx] = (1 << 31) | (tile_idx << 28) | (sub_idx << 25) | (z << 22) | (y << 12) | (x << 2) | palette_idx;
}

void set_background_palette(uint8_t palette_idx, uint32_t* data) {
    uint32_t dst = (uint32_t*)((char*)BACKGROUND_PALETTE[palette_idx*0x400])
    memcpy(dst, data, 0x400);
}

// controls for small sprites
void set_small_sprite_data(uint8_t sprite_idx, uint8_t* data){
    char* dst = ((char*)SMALL_SPRITE_DATA)[sprite_idx*0x100];
    memcpy(dst, data, 0x100);
}

void set_small_sprite_control(
    uint8_t sprite_ctrl_idx,
    uint8_t sprite_data_idx,
    uint16_t x,
    uint16_t y,
    uint16_t z,
    uint8_t palette_idx,
){
    SMALL_SPRITE_CONTROL[sprite_ctrl_idx] = (sprite_data_idx << 24) | (z << 21) | (y << 12) | (x << 2) | palette_idx;
}

void set_small_sprite_palette(uint8_t palette_idx, uint32_t* data) {
    uint32_t dst = (uint32_t*)((char*)SMALL_SPRITE_PALETTE[palette_idx*0x400])
    memcpy(dst, data, 0x400);
}

// controls for medium sprites
void set_medium_sprite_data(uint8_t sprite_idx, uint8_t* data){
    char* dst = ((char*)MEDIUM_SPRITE_DATA)[sprite_idx*0x400];
    memcpy(dst, data, 0x400);
}

void set_medium_sprite_control(
    uint8_t sprite_ctrl_idx,
    uint8_t sprite_data_idx,
    uint16_t x,
    uint16_t y,
    uint16_t z,
    uint8_t palette_idx,
){
    MEDIUM_SPRITE_CONTROL[sprite_ctrl_idx] = (sprite_data_idx << 24) | (z << 21) | (y << 12) | (x << 2) | palette_idx;
}

void set_medium_sprite_palette(uint8_t palette_idx, uint32_t* data) {
    uint32_t dst = (uint32_t*)((char*)MEDIUM_SPRITE_PALETTE[palette_idx*0x400])
    memcpy(dst, data, 0x400);
}

// controls for large sprites
void set_large_sprite_data(uint8_t sprite_idx, uint8_t* data){
    char* dst = ((char*)LARGE_SPRITE_DATA)[sprite_idx*0x1000];
    memcpy(dst, data, 0x1000);
}

void set_large_sprite_control(
    uint8_t sprite_ctrl_idx,
    uint8_t sprite_data_idx,
    uint16_t x,
    uint16_t y,
    uint16_t z,
    uint8_t palette_idx,
){
    LARGE_SPRITE_CONTROL[sprite_ctrl_idx] = (sprite_data_idx << 24) | (z << 21) | (y << 12) | (x << 2) | palette_idx;
}

void set_large_sprite_palette(uint8_t palette_idx, uint32_t* data) {
    uint32_t dst = (uint32_t*)((char*)LARGE_SPRITE_PALETTE[palette_idx*0x400])
    memcpy(dst, data, 0x400);
}

void switch_mode(uint32_t mode){
    MODE_CONTROL = (mode == 0) ? TEXT_MODE : GRAPHICS_MODE;
}

void simple_display_text(char *new_text, uint32_t start_idx){
    uint32_t bounds  = 0x900>>2;

    uint32_t offset = 0;
    while (new_text[offset] != "\0" && start_idx + offset < bounds){
        TEXT_DATA[start_idx + offset] = new_text[offset++];
    }
}

