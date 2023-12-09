#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Define TEXT_MODE and GRAPHICS_MODE
#define TEXT_MODE 0x0
#define GRAPHICS_MODE 0x1

typedef struct POS_TYPE
{
  int16_t x, y, z;
} POS_TYPE;

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
  SET_TILE_BACKGROUND = 21
};

/*Reading controller input*/
uint32_t CONTROLLER_PARAMS[] = {GET_CONTROLLER_REGISTER};
uint32_t CMD_PARAMS[] = {GET_CMD_STATUS};
uint32_t TIME_PARAMS[] = {GET_TIMER_TICKS};
uint32_t INTERRUPT_PARAMS[] = {GET_INTERRUPT_PENDING_REGISTER};
uint32_t VID_PARAMS[] = {GET_VID_PENDING};
uint32_t MODE_PARAMS[] = {GET_MODE_CONTROL_REGISTER};
uint32_t SWITCH_GRAPHICS_PARAMS[] = {SWITCH_MODE, GRAPHICS_MODE};
uint32_t SWITCH_TEXT_PARAMS[] = {SWITCH_MODE, TEXT_MODE};
uint32_t GLOBAL_TIME_PARAMS[] = {GET_CONTROLLER_REGISTER};
uint32_t INIT_THREAD_PARAMS[] = {INIT_THREAD};
uint32_t SWITCH_THREAD_PARAMS[] = {SWITCH_THREAD};
uint32_t INIT_MUTEX_PARAMS[] = {INIT_MUTEX};
uint32_t MUTEX_LOCK_PARAMS[] = {MUTEX_LOCK};
uint32_t MUTEX_UNLOCK_PARAMS[] = {MUTEX_UNLOCK};

typedef void (*TThreadEntry)(void *);
typedef uint32_t *TThreadContext;
typedef struct
{
  volatile bool locked;
} my_mutex_t;

uint32_t SystemCall(uint32_t *param);
uint32_t SystemCall2(uint32_t *param1, char *param2);
void switch_graphics_mode();
void switch_text_mode();
uint8_t is_controller_key_pessed(uint8_t key_idx);
uint32_t get_controller_status();
uint32_t my_rand(int range);
void set_medium_sprite(const POS_TYPE *pos, const uint8_t *sprite_data, const uint32_t *palette_data, const uint8_t CONTROL_IDX, const uint8_t PALETTE_IDX, const uint8_t DATA_IDX);
void set_large_sprite(const POS_TYPE *pos, const uint8_t *sprite_data, const uint32_t *palette_data, const uint8_t CONTROL_IDX, const uint8_t PALETTE_IDX, const uint8_t DATA_IDX);
void set_small_sprite(const POS_TYPE *pos, const uint8_t *sprite_data, const uint32_t *palette_data, const uint8_t CONTROL_IDX, const uint8_t PALETTE_IDX, const uint8_t DATA_IDX);
uint32_t get_time();
uint32_t get_global();
uint32_t get_interrupt();
uint32_t get_vid_bit();
void display_text(const char *new_text, const uint32_t start_idx);
uint32_t get_controller_status();
uint32_t get_cmd_status();
uint8_t is_controller_key_pessed(uint8_t key_idx);
TThreadContext InitThread(uint32_t *stacktop, TThreadEntry entry, void *param);
void SwitchThread(TThreadContext *oldcontext, TThreadContext newcontext);
void error_handling();
void my_mutex_init(my_mutex_t *mutex);
void my_mutex_lock(my_mutex_t *mutex);
void my_mutex_unlock(my_mutex_t *mutex);

int main()
{
  return 0;
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

uint32_t my_rand(int range)
{
  return rand() % range;
}

void set_medium_sprite(const POS_TYPE *pos, const uint8_t *sprite_data, const uint32_t *palette_data, const uint8_t CONTROL_IDX, const uint8_t PALETTE_IDX, const uint8_t DATA_IDX)
{
  switch_graphics_mode();
  uint32_t MEDIUM_SPRITE_PARAMS[] = {SET_MEDIUM_SPRITE, sprite_data, CONTROL_IDX, DATA_IDX, pos->x, pos->y, pos->z, PALETTE_IDX, palette_data};
  SystemCall(MEDIUM_SPRITE_PARAMS);
}

void set_small_sprite(const POS_TYPE *pos, const uint8_t *sprite_data, const uint32_t *palette_data, const uint8_t CONTROL_IDX, const uint8_t PALETTE_IDX, const uint8_t DATA_IDX)
{
  switch_graphics_mode();
  uint32_t SMALL_SPRITE_PARAMS[] = {SET_SMALL_SPRITE, sprite_data, CONTROL_IDX, DATA_IDX, pos->x, pos->y, pos->z, PALETTE_IDX, palette_data};
  SystemCall(SMALL_SPRITE_PARAMS);
}

void set_large_sprite(const POS_TYPE *pos, const uint8_t *sprite_data, const uint32_t *palette_data, const uint8_t CONTROL_IDX, const uint8_t PALETTE_IDX, const uint8_t DATA_IDX)
{
  switch_graphics_mode();
  uint32_t LARGE_SPRITE_PARAMS[] = {SET_LARGE_SPRITE, sprite_data, CONTROL_IDX, DATA_IDX, pos->x, pos->y, pos->z, PALETTE_IDX, palette_data};
  SystemCall(LARGE_SPRITE_PARAMS);
}

void set_pixel_background(const POS_TYPE *pos, const uint8_t *background_data, const uint32_t *palette_data, const uint8_t CONTROL_IDX, const uint8_t PALETTE_IDX, const uint8_t DATA_IDX)
{
  switch_graphics_mode();
  uint32_t BACKGROUND_PARAMS[] = {SET_BACKGROUND, DATA_IDX, background_data, CONTROL_IDX, pos->x, pos->y, pos->z, PALETTE_IDX, palette_data};
  SystemCall(BACKGROUND_PARAMS);
}

uint32_t get_time()
{
  return SystemCall(TIME_PARAMS);
}

uint32_t get_global()
{
  return SystemCall(GLOBAL_TIME_PARAMS);
}

uint32_t get_interrupt()
{
  return SystemCall(INTERRUPT_PARAMS);
}

uint32_t get_vid_bit()
{
  return SystemCall(VID_PARAMS);
}

void display_text(const char *new_text, const uint32_t start_idx)
{
  uint32_t DISPLAY_PARAMS[] = {DISPLAY_TEXT, start_idx};
  SystemCall2(DISPLAY_PARAMS, start_idx);
}

uint32_t get_controller_status()
{
  return SystemCall(CONTROLLER_PARAMS);
}

uint32_t get_cmd_status()
{
  return SystemCall(CMD_PARAMS);
}

// for AWXD keys only
uint8_t is_controller_key_pessed(uint8_t key_idx)
{
  uint32_t GET_CONTROLLER_KEY_PARAMS[] = {GET_CONTROLLER_KEY_STATUS, key_idx};
  return SystemCall(GET_CONTROLLER_KEY_PARAMS);
}

TThreadContext InitThread(uint32_t *stacktop, TThreadEntry entry, void *param)
{
  uint32_t INIT_THREAD_PARAMS[] = {INIT_THREAD, stacktop, entry, param};
  return SystemCall(INIT_THREAD_PARAMS);
}

void SwitchThread(TThreadContext *oldcontext, TThreadContext newcontext)
{
  uint32_t SWITCH_THREAD_PARAMS[] = {SWITCH_THREAD, oldcontext, newcontext};
  SystemCall(SWITCH_THREAD_PARAMS);
}

void error_handling()
{
  uint32_t ERROR_HANDLER_PARAMS[] = {ERROR_HANDLER_OPERATION};
  uint32_t error_code = SystemCall(ERROR_HANDLER_PARAMS); // error_code can be displayed by display_text
}

void my_mutex_init(my_mutex_t *mutex)
{
  uint32_t INIT_MUTEX_PARAMS[] = {INIT_MUTEX, mutex};
  SystemCall(INIT_MUTEX_PARAMS);
}

void my_mutex_lock(my_mutex_t *mutex)
{
  uint32_t MUTEX_LOCK_PARAMS[] = {MUTEX_LOCK, mutex};
  SystemCall(MUTEX_LOCK_PARAMS);
}

void my_mutex_unlock(my_mutex_t *mutex)
{
  uint32_t MUTEX_UNLOCK_PARAMS[] = {MUTEX_UNLOCK, mutex};
  SystemCall(MUTEX_UNLOCK_PARAMS);
}
