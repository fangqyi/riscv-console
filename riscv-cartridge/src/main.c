#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// Error handler
#include "../../riscv-firmware/src/error.h"

// Define TEXT_MODE and GRAPHICS_MODE
#define TEXT_MODE 0x0
#define GRAPHICS_MODE 0x1

/* Graphics parameters */
#define WIDTH 0x40                                          // Grid width of 0x40
#define HEIGHT 0x23                                         // Grid height of 0x23
#define SPRITE_DATA_SIZE 256                                // Size of sprite data in bytes
#define CONTROL_SIZE 4                                      // Size of control data in bytes
#define TOTAL_SPRITE_SIZE (SPRITE_DATA_SIZE + CONTROL_SIZE) // Total size of sprite and control data
#define PALETTE_SIZE 256
#define CONTROLLER (*((volatile uint32_t *)0x40000018))

uint32_t SystemCall(uint64_t *param);
uint32_t SystemCall2(uint64_t *param1, char *param2);
void display_video_clock_period();

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

/* Graphics function */
uint32_t CalculatePremultipliedRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  if (a != 255)
  {
    r = (r * a) / 255;
    g = (g * a) / 255;
    b = (b * a) / 255;
  }
  return (a << 24) | (b << 16) | (g << 8) | r;
}

volatile int global = 42;
volatile uint32_t controller_status = 0;

volatile int REFRESH_COUNT = 10000;

volatile uint32_t *MODE_REGISTER = (volatile uint32_t *)(0x500F6780);

int main()
{
  int last_global = 42;
  int text_pos = 12;
  int countdown = 1;

  /* Graphics parameters */
  uint16_t graphics_x_pos = 3;
  uint16_t graphics_y_pos = 3;
  uint16_t graphics_z_pos = 0;
  uint8_t *sprite_data = (uint8_t *)malloc(TOTAL_SPRITE_SIZE * sizeof(uint8_t));
  uint8_t *clear_data = (uint8_t *)malloc(TOTAL_SPRITE_SIZE * sizeof(uint8_t));
  uint32_t *palette_data = (uint32_t *)malloc(PALETTE_SIZE * sizeof(uint32_t));

  // Initialize Graphics sprite data
  for (int i = 0; i < SPRITE_DATA_SIZE; ++i)
  {
    sprite_data[i] = 255;
  }
  for (int i = 0; i < SPRITE_DATA_SIZE; ++i)
  {
    clear_data[i] = 0;
  }
  for (int i = 0; i < PALETTE_SIZE; ++i)
  {
    palette_data[i] = CalculatePremultipliedRGBA(255, 192, 146, 255);
  }

  uint32_t current_mode = 0;

  /*Reading controller input*/
  uint64_t CONTROLLER_PARAMS[] = {GET_CONTROLLER_REGISTER};
  uint64_t TIME_PARAMS[] = {GET_TIMER_TICKS};
  uint64_t MODE_PARAMS[] = {GET_MODE_CONTROL_REGISTER};
  uint64_t SWITCH_GRAPHICS_PARAMS[] = {SWITCH_MODE, GRAPHICS_MODE};
  uint64_t SWITCH_TEXT_PARAMS[] = {SWITCH_MODE, TEXT_MODE};

  SystemCall(SWITCH_GRAPHICS_PARAMS);
  uint8_t sprite_idx = 0;      // 0~255 FIXEME
  uint8_t sprite_ctrl_idx = 0; // 0~255
  uint8_t sprite_data_idx = 0; // 0~255
  uint8_t palette_idx = 0;     // 0~3

  //uint64_t SMALL_SPRITES_PARAMS2[] = {
  //    SET_SMALL_SPRITE,
  //    0, 0, 0};
  //SystemCall(SMALL_SPRITES_PARAMS2); // Draws a Small Sprite
  int r = 0;
  int g = 0;
  int b = 0;
  controller_status = CONTROLLER;

  while (1)
  {
    if (global != last_global){
      if (controller_status){
        // SystemCall(SMALL_SPRITES_test_PARAMS2);
        current_mode = SystemCall(MODE_PARAMS);
        if (current_mode != GRAPHICS_MODE)
        {
          SystemCall(SWITCH_GRAPHICS_PARAMS);
        }
        if (controller_status & 0x1)
        { // 'd' -> LEFT
         uint64_t SMALL_SPRITES_test_PARAMS2[] = {SET_SMALL_SPRITE, 0, 0, 0};
         SystemCall(SMALL_SPRITES_test_PARAMS2);   
          //r = 1; g = 0; b = 0; 
        }
        if (controller_status & 0x2)
        { // 'w' -> UP
         uint64_t SMALL_SPRITES_test_PARAMS2[] = {SET_SMALL_SPRITE, 10, 10, 0};
        SystemCall(SMALL_SPRITES_test_PARAMS2); 
         // r = 2; g = 225; b = 100; 
        }
        if (controller_status & 0x4)
        { // 'x' -> DOWN
         uint64_t SMALL_SPRITES_test_PARAMS2[] = {SET_SMALL_SPRITE, 5, 5, 0};
         SystemCall(SMALL_SPRITES_test_PARAMS2); 
          //r = 3; g = 250; b = 25; 
        }
        if (controller_status & 0x8)
        { // 'a' -> RIGHT
          //r = 4; g = 20; b = 250; 
           uint64_t SMALL_SPRITES_test_PARAMS2[] = {SET_SMALL_SPRITE, 15, 15, 0};
          SystemCall(SMALL_SPRITES_test_PARAMS2); 
          //display_video_clock_period();
        }
        //uint64_t SMALL_UPDATED_SPRITES_PARAMS2[] = {SET_SMALL_SPRITE, (x_pos & 0x3F) << 3, (x_pos >> 6) << 3, 0};
        //uint64_t SMALL_UPDATED_SPRITES_PARAMS2[] = {SET_SMALL_SPRITE,r, g, b};
        //SystemCall(SMALL_UPDATED_SPRITES_PARAMS2);   
      }
      last_global = global;
    }
    countdown--;
    if (!countdown)
    {
      global++;
      controller_status = CONTROLLER; // SystemCall(CONTROLLER_PARAMS);
      countdown = REFRESH_COUNT;

      // Display the video clock period
      display_video_clock_period();
    }
  }
  return 0;
}
// Function to read and display video clock period
void display_video_clock_period()
{
  uint64_t MODE_PARAMS[] = {GET_MODE_CONTROL_REGISTER};
  uint64_t SWITCH_TEXT_PARAMS[] = {SWITCH_MODE, TEXT_MODE};
  uint32_t current_mode = SystemCall(MODE_PARAMS);
  if (current_mode != TEXT_MODE)
  {
    SystemCall(SWITCH_TEXT_PARAMS);
  }

  uint64_t TIME_PARAMS[] = {GET_TIMER_TICKS};
  uint32_t period = SystemCall(TIME_PARAMS);
  uint64_t TIME_DISPLAY_PARAMS[] = {DISPLAY_TEXT, 41};
  // Convert the period integer to a string and print it to VIDEO_MEMORY
  char periodStr[40]; //
  snprintf(periodStr, sizeof(periodStr), "Video Clock Period: %d ms", period);
  SystemCall2(TIME_DISPLAY_PARAMS, periodStr);
}
