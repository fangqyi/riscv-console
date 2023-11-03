#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

volatile int REFRESH_COUNT = 100000;

int main()
{
  int last_global = 42;
  int text_pos = 12;
  int countdown = 1;

  /* Graphics parameters */
  uint16_t graphics_x_pos = 50;
  uint16_t graphics_y_pos = 50;
  uint16_t graphics_z_pos = 3;
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

  uint64_t SMALL_SPRITES_PARAMS2[] = {SET_SMALL_SPRITE,
                                      sprite_idx,
                                      sprite_data,
                                      sprite_ctrl_idx,
                                      sprite_data_idx,
                                      graphics_x_pos,
                                      graphics_y_pos,
                                      graphics_z_pos,
                                      palette_idx,
                                      palette_data};
  SystemCall(SMALL_SPRITES_PARAMS2); // Draws a Small Sprite

  controller_status = SystemCall(CONTROLLER_PARAMS);
  while (1)
  {
    if (global != last_global)
    {
      if (controller_status)
      {
        current_mode = SystemCall(MODE_PARAMS);
        if (current_mode != GRAPHICS_MODE)
        {
          SystemCall(SWITCH_GRAPHICS_PARAMS);
        }
        if (current_mode == GRAPHICS_MODE)
        {
          // uint32_t CLS_PARAMS[] = {DISPLAY_TEXT, init_text_pos};
          // char clear_text[] = " ";
          // SystemCall2(CLS_PARAMS, clear_text);

          // Clear original sprite
          uint64_t SMALL_SPRITES_PARAMS2[] = {SET_SMALL_SPRITE,
                                              sprite_idx,
                                              clear_data,
                                              sprite_ctrl_idx,
                                              sprite_data_idx,
                                              graphics_x_pos,
                                              graphics_y_pos,
                                              graphics_z_pos,
                                              palette_idx,
                                              palette_data};
          SystemCall(SMALL_SPRITES_PARAMS2);

          if (controller_status & 0x1)
          { // 'd' -> LEFT
            if (graphics_x_pos > 0)
            {
              graphics_x_pos--;
            }
          }
          if (controller_status & 0x2)
          { // 'w' -> UP
            if (graphics_y_pos > 0)
            {
              graphics_y_pos--;
            }
          }
          if (controller_status & 0x4)
          { // 'x' -> DOWN
            if (graphics_y_pos < (HEIGHT - 1))
            {
              graphics_y_pos++;
            }
          }
          if (controller_status & 0x8)
          { // 'a' -> RIGHT
            if (graphics_x_pos < (WIDTH - 1))
            {
              graphics_x_pos++;
            }
          }

          uint64_t SMALL_UPDATED_SPRITES_PARAMS2[] = {SET_SMALL_SPRITE,
                                                      sprite_idx,
                                                      sprite_data,
                                                      sprite_ctrl_idx,
                                                      sprite_data_idx,
                                                      graphics_x_pos,
                                                      graphics_y_pos,
                                                      graphics_z_pos,
                                                      palette_idx,
                                                      palette_data};
          SystemCall(SMALL_UPDATED_SPRITES_PARAMS2);
        }
        last_global = global;
      }
      countdown--;
      if (!countdown)
      {
        global++;
        controller_status = SystemCall(CONTROLLER_PARAMS);
        countdown = REFRESH_COUNT;

        // Display the video clock period
        display_video_clock_period();

        // Switch between text and graphics mode based on global value
        // uint32_t syscall_params_switch[2];
        // int32_t MODE_PARAMS[1];
        // MODE_PARAMS[0] = GET_MODE_CONTROL_REGISTER; // GET_MODE_CONTROL_REGISTER syscall number
        // syscall_params_switch[0] = SWITCH_MODE;     // SWITCH_MODE syscall number
        // syscall_params_switch[1] = (global % 2) ? GRAPHICS_MODE : TEXT_MODE; // Alternate between graphics and text mode
        // SystemCall(syscall_params_switch); // Switch mode
        //  c_syscall(syscall_params, NULL); // Switch mode
        // current_mode = SystemCall(MODE_PARAMS); // Get the mode status
      }
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

  char ClockPeriodMsg[] = "Video Clock Period: ";

  // Copy the message
  uint64_t TIME_DISPLAY_PARAMS[] = {DISPLAY_TEXT, 41};
  SystemCall2(TIME_DISPLAY_PARAMS, ClockPeriodMsg);
  // Convert the period integer to a string and print it to VIDEO_MEMORY
  char periodStr[10]; //
  snprintf(periodStr, sizeof(periodStr), "%d ms", period);

  TIME_DISPLAY_PARAMS[1] = 61;
  SystemCall2(TIME_DISPLAY_PARAMS, periodStr);
}