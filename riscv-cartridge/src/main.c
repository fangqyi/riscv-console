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
// #define SMALL_SPRITE_SIZE 8
#define PLAYER_1_SPRITE_DATA_IDX 1
#define PLAYER_1_SPRITE_PALETTE_IDX 1
#define PLAYER_1_SPRITE_CONTROL_IDX 1
#define PLAYER_2_SPRITE_DATA_IDX 2
#define PLAYER_2_SPRITE_PALETTE_IDX 2
#define PLAYER_2_SPRITE_CONTROL_IDX 2
#define TARGET_SPRITE_DATA_IDX 3
#define TARGET_SPRITE_PALETTE_IDX 3
#define TARGET_SPRITE_CONTROL_IDX 3
#define TIME_SPRITE_DATA_IDX 4
#define TIME_SPRITE_PALETTE_IDX 4
#define TIME_SPRITE_CONTROL_IDX 4
#define BG_CONTROL_IDX 0
#define BG_PIXEL_DATA_IDX 0
#define BG_PALETTE_IDX 0
#define BG_IMAGE_IDX 0
#define COL_MAGIC_LIMIT 460
#define ROW_MAGIC_LIMIT 230
#define LARGE_MAGIC_SIZE 60
uint32_t RED = 0xFFFF0000;
uint32_t BLUE = 0xFF3377FF;
uint32_t GREEN = 0xFF33DD33;
uint32_t DARK_GREEN = 0xFF006600;
uint32_t WHITE = 0xFFFFFFFF;
uint32_t YELLOW = 0xFFFFFF00;
uint32_t ORANGE = 0xFFFF7F50;
uint32_t DEEP_PINK = 0xFFFF1493;
uint32_t LIGHT_PINK = 0xFFFFB6C1;
uint32_t HOT_PINK = 0xFFFF69B4;

typedef struct POS_TYPE
{
  int16_t x, y, z;
} POS_TYPE;

uint32_t SystemCall(uint32_t *param);
uint32_t SystemCall2(uint32_t *param1, char *param2);
void set_player_sprite_1(POS_TYPE *pos);
void set_target_sprite1(POS_TYPE *pos);
void set_target_sprite2(POS_TYPE *pos);
void set_background();
void control_movement_1(POS_TYPE *player_pos);
uint32_t display_time_remain(uint32_t start_time);
void wait_for_any_key_input();
void wait_for_i_key_input();
void show_start_screen();
uint32_t game_loop();
void show_end_screen(uint32_t score);
uint32_t score_update_1(uint32_t score, const POS_TYPE *player_pos, POS_TYPE *target_pos1, POS_TYPE *target_pos2);
uint32_t my_rand(int range);
void switch_graphics_mode();
void switch_text_mode();
uint8_t is_controller_key_pessed(uint8_t key_idx);
uint32_t get_controller_status();
void display_key_test(uint32_t key);

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
  ERROR_HANDLER_OPERATION = 11,
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

volatile uint32_t controller_status = 0;
void custom_delay(int milliseconds);

/*Reading controller input*/
uint32_t CONTROLLER_PARAMS[] = {GET_CONTROLLER_REGISTER};
uint32_t CMD_PARAMS[] = {GET_CMD_STATUS};
uint32_t TIME_PARAMS[] = {GET_TIMER_TICKS};
uint32_t MODE_PARAMS[] = {GET_MODE_CONTROL_REGISTER};
uint32_t SWITCH_GRAPHICS_PARAMS[] = {SWITCH_MODE, GRAPHICS_MODE};
uint32_t SWITCH_TEXT_PARAMS[] = {SWITCH_MODE, TEXT_MODE};
uint32_t GLOBAL_TIME_PARAMS[] = {GET_CONTROLLER_REGISTER};

int main()
{
  show_start_screen();
  wait_for_any_key_input();
  set_background();
  while (1)
  {
    uint32_t score = 0;
    score = game_loop();
    show_end_screen(score);
    custom_delay(1000);
    wait_for_any_key_input();
    // wait_for_i_key_input(); // Wait here until 'i' key is pressed
  }
  return 0;
}

uint32_t game_loop()
{
  int32_t score = 0;
  int32_t time_limit = 60000; // 60 seconds (firmware)

  POS_TYPE player_1_pos = {.x = 0, .y = 0, .z = 0};
  POS_TYPE target_pos1 = {.x = my_rand(COL_MAGIC_LIMIT), .y = my_rand(ROW_MAGIC_LIMIT), .z = 3};
  POS_TYPE target_pos2 = {.x = my_rand(COL_MAGIC_LIMIT), .y = my_rand(ROW_MAGIC_LIMIT), .z = 3};

  set_player_sprite_1(&player_1_pos);
  set_target_sprite1(&target_pos1);
  set_target_sprite2(&target_pos2);

  uint32_t start_time = SystemCall(TIME_PARAMS); // Start timer

  while (SystemCall(TIME_PARAMS) - start_time < time_limit)
  {
    controller_status = get_controller_status();
    if (controller_status)
    {
      control_movement_1(&player_1_pos);
      set_player_sprite_1(&player_1_pos);
      score = score_update_1(score, &player_1_pos, &target_pos1, &target_pos2);
    }

    // display_time_remain(start_time); // Update the countdown timer
  }

  return score; // Return the score after time limit
}

void switch_graphics_mode()
{
  if (SystemCall(MODE_PARAMS) != GRAPHICS_MODE)
  {
    SystemCall(SWITCH_GRAPHICS_PARAMS);
  }
}

void switch_text_mode()
{
  if (SystemCall(MODE_PARAMS) != TEXT_MODE)
  {
    SystemCall(SWITCH_TEXT_PARAMS);
  }
}

uint32_t score_update_1(uint32_t score, const POS_TYPE *player_pos, POS_TYPE *target_pos1, POS_TYPE *target_pos2)
{
  if ((player_pos->x + LARGE_MAGIC_SIZE >= target_pos1->x) && (player_pos->x <= target_pos1->x + LARGE_MAGIC_SIZE) &&
      (player_pos->y + LARGE_MAGIC_SIZE >= target_pos1->y) && (player_pos->y <= target_pos1->y + LARGE_MAGIC_SIZE))
  {
    /* Update score and position of target point if eaten */
    score++;
    target_pos1->x = my_rand(COL_MAGIC_LIMIT);
    target_pos1->y = my_rand(ROW_MAGIC_LIMIT);
    set_target_sprite1(target_pos1); // Draw new target
  }
  if ((player_pos->x + LARGE_MAGIC_SIZE >= target_pos2->x) && (player_pos->x <= target_pos2->x + LARGE_MAGIC_SIZE) &&
      (player_pos->y + LARGE_MAGIC_SIZE >= target_pos2->y) && (player_pos->y <= target_pos2->y + LARGE_MAGIC_SIZE))
  {
    /* Update score and position of target point if eaten */
    score++;
    target_pos2->x = my_rand(COL_MAGIC_LIMIT);
    target_pos2->y = my_rand(ROW_MAGIC_LIMIT);
    set_target_sprite2(target_pos2); // Draw new target
  }
  return score;
}

uint32_t my_rand(int range)
{
  // uint32_t TIME_PARAMS[] = {GET_TIMER_TICKS};
  // uint32_t rand = SystemCall(TIME_PARAMS) % range;
  return rand() % range;
}

void control_movement_1(POS_TYPE *player_pos)
{
  if (controller_status & 0x1)
  { // 'a' -> LEFT'
    player_pos->x -= LARGE_MAGIC_SIZE / 30;
    if (player_pos->x <= 0)
    {
      player_pos->x = COL_MAGIC_LIMIT;
    }
  }
  if (controller_status & 0x2)
  { // 'w' -> UP
    player_pos->y -= LARGE_MAGIC_SIZE / 30;
    if (player_pos->y <= 0)
    {
      player_pos->y = ROW_MAGIC_LIMIT;
    }
  }
  if (controller_status & 0x4)
  { // 'x' -> DOWN
    player_pos->y += LARGE_MAGIC_SIZE / 30;
    if (player_pos->y >= ROW_MAGIC_LIMIT)
    {
      player_pos->y = 0;
    }
  }
  if (controller_status & 0x8)
  { // d' -> RIGHT
    player_pos->x += LARGE_MAGIC_SIZE / 30;
    if (player_pos->x >= COL_MAGIC_LIMIT)
    {
      player_pos->x = 0;
    }
  }
}

void set_player_sprite_1(POS_TYPE *pos)
{
  uint8_t sprite_data[0x1000];
  uint32_t palette_data[0x100];
  for (int i = 0; i < 0x20; i++)
  {
    for (int j = 0; j < 0x20; j++)
    {
      palette_data[(i * 0x50 + j) % 0x100] = 0;
      sprite_data[i * 0x20 + j] = (i + j) % 5;
    }
  }
  palette_data[0] = RED;
  palette_data[1] = WHITE;
  palette_data[2] = BLUE;
  palette_data[3] = GREEN;
  palette_data[4] = YELLOW;

  switch_graphics_mode();
  uint32_t LARGE_SPRITE_PARAMS[] = {SET_LARGE_SPRITE, sprite_data, PLAYER_1_SPRITE_CONTROL_IDX, PLAYER_1_SPRITE_DATA_IDX, pos->x, pos->y, pos->z, PLAYER_1_SPRITE_PALETTE_IDX, palette_data};
  SystemCall(LARGE_SPRITE_PARAMS);
}

void set_target_sprite1(POS_TYPE *pos)
{
  uint8_t sprite_data[0x400];
  uint32_t palette_data[0x100];
  for (int i = 0; i < 0x20; i++)
  {
    for (int j = 0; j < 0x20; j++)
    {
      palette_data[(i * 0x20 + j) % 0x100] = 0;

      sprite_data[i * 0x20 + j] = my_rand(3); // i < 0x10 ? (j < 0x10 ? 0 : 1) : (j < 0x10 ? 2 : 3);
    }
  }
  palette_data[0] = LIGHT_PINK;
  palette_data[1] = HOT_PINK;
  palette_data[2] = DEEP_PINK;

  switch_graphics_mode();
  uint32_t SMALL_SPRITE_PARAMS[] = {SET_SMALL_SPRITE, sprite_data, TARGET_SPRITE_CONTROL_IDX, TARGET_SPRITE_DATA_IDX, pos->x, pos->y, pos->z, TARGET_SPRITE_PALETTE_IDX, palette_data};

  // uint32_t MEDIUM_SPRITE_PARAMS[] = {SET_MEDIUM_SPRITE, sprite_data, TARGET_SPRITE_CONTROL_IDX, TARGET_SPRITE_DATA_IDX, pos->x, pos->y, pos->z, TARGET_SPRITE_PALETTE_IDX, palette_data};
  SystemCall(SMALL_SPRITE_PARAMS);
}

void set_target_sprite2(POS_TYPE *pos)
{
  uint8_t sprite_data[0x400];
  uint32_t palette_data[0x100];
  for (int i = 0; i < 0x20; i++)
  {
    for (int j = 0; j < 0x20; j++)
    {
      palette_data[(i * 0x20 + j) % 0x100] = 0;

      sprite_data[i * 0x20 + j] = my_rand(3); // i < 0x10 ? (j < 0x10 ? 0 : 1) : (j < 0x10 ? 2 : 3);
    }
  }
  palette_data[0] = LIGHT_PINK;
  palette_data[1] = HOT_PINK;
  palette_data[2] = DEEP_PINK;

  switch_graphics_mode();
  uint32_t MEDIUM_SPRITE_PARAMS[] = {SET_MEDIUM_SPRITE, sprite_data, TARGET_SPRITE_CONTROL_IDX, TARGET_SPRITE_DATA_IDX, pos->x, pos->y, pos->z, TARGET_SPRITE_PALETTE_IDX, palette_data};

  SystemCall(MEDIUM_SPRITE_PARAMS);
}

void set_background()
{
  uint8_t background_data[0x24000];
  uint32_t palette_data[0x100];
  for (int i = 0; i < 0x10; i++)
  {
    for (int j = 0; j < 0x10; j++)
    {
      palette_data[(i * 0x20 + j) % 0x100] = 0;
    }
  }
  for (int i = 0; i < 0x24000; i++)
  {
    background_data[i] = i % 16 < 8 ? 0 : 1;
  }
  palette_data[0] = DARK_GREEN;
  palette_data[1] = GREEN;

  switch_graphics_mode();
  uint32_t BG_PARAMS[] = {SET_BACKGROUND, BG_IMAGE_IDX, background_data, BG_CONTROL_IDX, BG_PIXEL_DATA_IDX, 0, 0, 0, BG_PALETTE_IDX, palette_data};
  SystemCall(BG_PARAMS);
  // int32_t TIME_PARAMS[] = {GET_TIMER_TICKS};
  // uint32_t start_time = SystemCall(TIME_PARAMS);
  // display_time_remain(start_time);
}

uint32_t display_time_remain(uint32_t start_time)
{
  uint32_t current_time = SystemCall(TIME_PARAMS);
  uint32_t elapsed_time = (current_time - start_time) / 1000; // Assuming 1000 ticks is 1 second
  uint32_t remaining_time = 60 - elapsed_time;

  // Display the remaining time
  char time_str[40];
  snprintf(time_str, sizeof(time_str), "Time remaining: %d s", remaining_time);
  // Here you need to implement the logic to display this string in your game's graphics mode.
  // For now, assuming it's a simple text display:
  switch_text_mode();
  uint32_t TIME_DISPLAY_PARAMS[] = {DISPLAY_TEXT, 41};
  SystemCall2(TIME_DISPLAY_PARAMS, time_str);

  return remaining_time;
}



void custom_delay(int milliseconds)
{
  uint32_t end_time = SystemCall(TIME_PARAMS) + milliseconds;
  while (SystemCall(TIME_PARAMS) < end_time)
    ;
}

void show_start_screen()
{
  uint32_t DISPLAY_PARAMS[] = {DISPLAY_TEXT, 0};
  // Convert the period integer to a string and print it to VIDEO_MEMORY
  char welcome_text[200]; //
  snprintf(welcome_text, sizeof(welcome_text), "Welcome! Excellent gamer. Please read the following game rule. Game rule: try to eat as many pink squares as possible in 60 secs. Move by pressing WADX keys. Good luck : ) Press W/A/D/X to start");
  SystemCall2(DISPLAY_PARAMS, welcome_text);
}

void show_end_screen(uint32_t score)
{
  /* Display final score and end game message*/
  switch_text_mode();
  uint32_t SCORE_DISPLAY_PARAMS[] = {DISPLAY_TEXT, 0};
  char end_text[200];
  snprintf(end_text, sizeof(end_text), "Game is over! YOU (player 1) GOT %d POINTS. Press W/A/X/D/U/I/J/K to continue                                                                                                                                                                                                                     ", score);
  SystemCall2(SCORE_DISPLAY_PARAMS, end_text);
}

void wait_for_any_key_input()
{
  while (get_controller_status() == 0x0)
    ;
}

void wait_for_i_key_input()
{
  while (get_controller_status() & 0x20)
    ;
}

uint32_t get_controller_status()
{
  return SystemCall(CONTROLLER_PARAMS);
}

uint32_t get_cmd_status()
{
  return SystemCall(CMD_PARAMS);
}
uint8_t is_controller_key_pessed(uint8_t key_idx)
{
  uint32_t GET_CONTROLLER_KEY_PARAMS[] = {GET_CONTROLLER_KEY_STATUS, key_idx};
  return SystemCall(GET_CONTROLLER_KEY_PARAMS);
}

void display_key_test(uint32_t key)
{
  switch_text_mode();
  uint32_t KEY_TEST_DISPLAY_PARAMS[] = {DISPLAY_TEXT, 41};
  char periodStr[40];
  snprintf(periodStr, sizeof(periodStr), "the 3 key is pressed ");
  SystemCall2(KEY_TEST_DISPLAY_PARAMS, periodStr);
}
