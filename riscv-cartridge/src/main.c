#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Define TEXT_MODE and GRAPHICS_MODE
#define TEXT_MODE 0x0
#define GRAPHICS_MODE 0x1

/* Graphics parameters */
#define WIDTH 512  // Grid width of 512
#define HEIGHT 288 // Grid height of 288
#define MEDIUM_SPRITE_SIZE 16
#define SMALL_SPRITE_SIZE 8
// #define CONTROL_SIZE 4                                      // Size of control data in bytes
// #define TOTAL_SPRITE_SIZE (SPRITE_DATA_SIZE + CONTROL_SIZE) // Total size of sprite and control data
// #define PALETTE_SIZE 256
uint32_t RED = 0xFFFF0000;
uint32_t BLUE = 0xFF3377FF;
uint32_t GREEN = 0xFF33DD33;
uint32_t WHITE = 0xFFFFFFFF;
uint32_t YELLOW = 0xFFFFFF00;
uint32_t ORANGE = 0xFFFF7F50;
#define CONTROLLER (*((volatile uint32_t *)0x40000018)) // TODO: rewrite with Syscall

uint32_t SystemCall(uint64_t *param);
uint32_t SystemCall2(uint64_t *param1, char *param2);
// void display_video_clock_period();
// void simple_medium_sprite(int16_t x, int16_t y, int16_t z);
void set_player_sprite(int16_t x, int16_t y, int16_t z);
void set_target_sprite(int16_t x, int16_t y, int16_t z);
int16_t control_movement_1(uint32_t controller_status, int16_t x_1, int16_t y_1, int16_t z_1);
uint32_t display_time_remain(uint32_t start_time);
// void showStartScreen();
int32_t game_loop();
void show_end_screen();
void wait_for_any_key_input();
int32_t score_update_1(int32_t score, int16_t x_1, int16_t y_1, int16_t target_x, int16_t target_y, int16_t target_z);
int16_t my_rand(int range);

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

volatile int global = 42;
volatile uint32_t controller_status = 0;

volatile int REFRESH_COUNT = 10000;

volatile uint32_t *MODE_REGISTER = (volatile uint32_t *)(0x500F6780); // TODO: rewrite with Syscall
/*Reading controller input*/
uint64_t CONTROLLER_PARAMS[] = {GET_CONTROLLER_REGISTER};
uint64_t TIME_PARAMS[] = {GET_TIMER_TICKS};
uint64_t MODE_PARAMS[] = {GET_MODE_CONTROL_REGISTER};
uint64_t SWITCH_GRAPHICS_PARAMS[] = {SWITCH_MODE, GRAPHICS_MODE};
uint64_t SWITCH_TEXT_PARAMS[] = {SWITCH_MODE, TEXT_MODE};

int main()
{
  uint32_t current_mode = 0;

  SystemCall(SWITCH_GRAPHICS_PARAMS);

  controller_status = CONTROLLER; // TODO: rewrite with Syscall

  while (1)
  {
    int32_t score = 0;
    // showStartScreen(); // Implement this function to show the start screen
    // wait_for_any_key_input(); // Implement this function to wait for user input to start
    score = game_loop();
    show_end_screen(score);
    wait_for_any_key_input(); // Implement this function to wait for user input to restart
  }
  return 0;
}

int32_t game_loop()
{
  uint32_t current_mode = 0;
  int countdown = 1;
  int last_global = 42;
  int32_t score = 0;
  int32_t remainingTime = 60; // 60 seconds
  // Initial position of player 1
  int16_t x_1 = 10;
  int16_t y_1 = 10;
  int16_t z_1 = 0;
  // Initial position of player 2
  int16_t x_2 = WIDTH - MEDIUM_SPRITE_SIZE - 10;
  int16_t y_2 = HEIGHT - MEDIUM_SPRITE_SIZE - 10;
  int16_t z_2 = 1;
  int16_t target_x = my_rand(WIDTH - SMALL_SPRITE_SIZE);
  int16_t target_y = my_rand(HEIGHT - SMALL_SPRITE_SIZE);
  int16_t target_z = 3;                            // z is always 3 for the target
  set_target_sprite(target_x, target_y, target_z); // Draw initial target

  uint64_t TIME_PARAMS[] = {GET_TIMER_TICKS};
  uint32_t start_time = SystemCall(TIME_PARAMS); // Start timer
  while (remainingTime > 0)
  {
    /* Handle user input for player movement*/
    if (global != last_global)
    {
      if (controller_status)
      {
        // SystemCall(SMALL_SPRITES_test_PARAMS2);
        current_mode = SystemCall(MODE_PARAMS);
        if (current_mode != GRAPHICS_MODE)
        {
          SystemCall(SWITCH_GRAPHICS_PARAMS);
        }
        x_1, y_1, z_1 = control_movement_1(controller_status, x_1, y_1, z_1);
        set_player_sprite_1(x_1, y_1, z_1);
      }
      last_global = global;
    }
    countdown--;
    if (!countdown)
    {
      global++;
      controller_status = CONTROLLER; // SystemCall(CONTROLLER_PARAMS);
      countdown = REFRESH_COUNT;

      /*Display the video clock period*/
      // display_video_clock_period();

      /* Update the countdown timer and check for time-up condition */
      remainingTime = display_time_remain(start_time);
    }
    /* Check if snake "eats" the target point*/
    /* Update score of player 1*/
    score = score_update_1(score, x_1, y_1, target_x, target_y, target_z);
  }
  return score;
}

int32_t score_update_1(int32_t score, int16_t x_1, int16_t y_1, int16_t target_x, int16_t target_y, int16_t target_z)
{
  if ((x_1 < target_x + SMALL_SPRITE_SIZE) && (x_1 + MEDIUM_SPRITE_SIZE > target_x) &&
      (y_1 < target_y + SMALL_SPRITE_SIZE) && (y_1 + MEDIUM_SPRITE_SIZE > target_y))
  {
    /* Update score and position of target point if eaten */
    score++;
    target_x = my_rand(WIDTH - SMALL_SPRITE_SIZE);
    target_y = my_rand(HEIGHT - SMALL_SPRITE_SIZE);
    set_target_sprite(target_x, target_y, target_z); // Draw new target
  }
  return score;
}

int16_t my_rand(int range)
{
  uint64_t TIME_PARAMS[] = {GET_TIMER_TICKS};
  int16_t rand = SystemCall(TIME_PARAMS) % range;
  return rand;
}
int16_t control_movement_1(uint32_t controller_status, int16_t x_1, int16_t y_1, int16_t z_1)
{
  if (controller_status & 0x1)
  { // 'd' -> LEFT
    x_1 -= 1;
    if (x_1 < 0)
    {
      x_1 = WIDTH - MEDIUM_SPRITE_SIZE;
    }
  }
  if (controller_status & 0x2)
  { // 'w' -> UP
    y_1 -= 1;
    if (y_1 < 0)
    {
      y_1 = HEIGHT - MEDIUM_SPRITE_SIZE;
    }
  }
  if (controller_status & 0x4)
  { // 'x' -> DOWN
    y_1 += 1;
    if (y_1 > HEIGHT - MEDIUM_SPRITE_SIZE)
    {
      y_1 = 0;
    }
  }
  if (controller_status & 0x8)
  { // 'a' -> RIGHT
    x_1 += 1;
    if (x_1 > (WIDTH - MEDIUM_SPRITE_SIZE))
    {
      x_1 = 0;
    }
  }
  return x_1, y_1, z_1;
}

void set_player_sprite_1(int16_t x, int16_t y, int16_t z)
{
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
  palette_data[0] = RED;
  palette_data[1] = WHITE;

  // set_medium_sprite_palette(2, palette_data);
  // set_medium_sprite_data(10, sprite_data);
  // set_medium_sprite_control(5, 10, x, y, z, 2);
  uint32_t MEDIUM_SPRITES_PARAMS[] = {SET_MEDIUM_SPRITE, 10, sprite_data, 5, 10, x, y, z, 2, palette_data};
  SystemCall(MEDIUM_SPRITES_PARAMS);
  /*
  {10, sprite_data, 5, 10, x, y, z, 2, palette_data}
  {uint8_t sprite_idx, uint8_t* data, uint8_t sprite_ctrl_idx, uint8_t sprite_data_idx, uint16_t x, uint16_t y, uint16_t z, uint8_t palette_idx, uint32_t* palette_data}
  set_medium_sprite_control(sprite_ctrl_idx, sprite_data_idx, x, y, z, palette_idx);
  set_medium_sprite_data(sprite_idx, data);
  set_medium_sprite_palette(palette_idx, palette_data);
  */
}
void set_target_sprite(int16_t x, int16_t y, int16_t z)
{
  uint8_t sprite_data[0x100];
  uint32_t palette_data[0x100];
  for (int i = 0; i < 0x10; i++)
  {
    for (int j = 0; j < 0x10; j++)
    {
      palette_data[(i * 0x10 + j) % 0x100] = 0;
      sprite_data[i * 0x10 + j] = i < 0x8 ? (j < 0x8 ? 0 : 1) : (j < 0x8 ? 2 : 3);
    }
  }
  palette_data[0] = ORANGE;
  palette_data[1] = GREEN;
  palette_data[2] = BLUE;
  palette_data[3] = YELLOW;

  // set_small_sprite_palette(1, palette_data);
  // set_small_sprite_data(10, sprite_data);
  // set_small_sprite_control(5, 10, x, y, z, 1);
  uint32_t SMALL_SPRITE_PARAMS[] = {SET_SMALL_SPRITE, 10, sprite_data, 5, 10, x, y, z, 2, palette_data};
  SystemCall(SMALL_SPRITE_PARAMS);
}

uint32_t display_time_remain(uint32_t start_time)
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
  snprintf(periodStr, sizeof(periodStr), "Time remain: %d s", 60 - (period - start_time) / 1000);
  SystemCall2(TIME_DISPLAY_PARAMS, periodStr);
  return 60 - (period - start_time) / 1000;
}

// void showStartScreen() {
//     // Display start game message
// }

void show_end_screen(int32_t score)
{
  /* Display final score and end game message*/
  uint64_t MODE_PARAMS[] = {GET_MODE_CONTROL_REGISTER};
  uint64_t SWITCH_TEXT_PARAMS[] = {SWITCH_MODE, TEXT_MODE};
  uint32_t current_mode = SystemCall(MODE_PARAMS);
  if (current_mode != TEXT_MODE)
  {
    SystemCall(SWITCH_TEXT_PARAMS);
  }

  // uint64_t TIME_PARAMS[] = {GET_TIMER_TICKS};
  // uint32_t period = SystemCall(TIME_PARAMS);
  uint64_t SCORE_DISPLAY_PARAMS[] = {DISPLAY_TEXT, 41};
  // Convert the period integer to a string and print it to VIDEO_MEMORY
  char periodStr[40]; //
  snprintf(periodStr, sizeof(periodStr), "GAME OVER!!!  PLAYER 1 GOT %d POINTS", score);
  SystemCall2(SCORE_DISPLAY_PARAMS, periodStr);
}

void wait_for_any_key_input()
{
  while (controller_status == 0x0);
}