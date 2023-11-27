#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdbool.h>


volatile uint32_t controller_status = 0;
volatile int global = 0;

// typedef struct {
//     volatile bool locked;
// } my_mutex_t;

// void my_mutex_init(my_mutex_t *mutex) {
//     mutex->locked = false;
// }

// void my_mutex_lock(my_mutex_t *mutex) {
//     while (true) {

//         if (!mutex->locked) {
//             mutex->locked = true;
//             break; // Successfully acquired the mutex
//         }

//     }
// }

// void my_mutex_unlock(my_mutex_t *mutex) {

//     mutex->locked = false;

// }

// cartridge setup
volatile uint32_t *CartridgeStatus = (volatile uint32_t *)(0x4000001C);
typedef void (*FunctionPtr)(void);
void simple_medium_sprite(uint16_t x, uint16_t y, uint16_t z);
void simple_display_text(char *new_text, uint32_t start_idx);
uint32_t get_controller_status_key(uint8_t key_idx);



int main()
{
  // my_mutex_t my_mutex;
  // my_mutex_init(&my_mutex);
  // simple_medium_sprite(0,0,0);
  // loading cartridge
  /*char buf[80];
  uint8_t data[0x100];
  uint8_t data2[0x10];
  uint8_t a = 0;
  uint32_t b=0;
  snprintf(buf, sizeof(buf), "ptr %d, ptr0x10 %d, uint8_t* %d, uint32_t %d", sizeof(data), sizeof(data2), sizeof(&a), sizeof(b));
  simple_display_text(buf, 0);*/
  while (1)
  {
    // int k = 1;
    // my_mutex_lock(&my_mutex);
    // simple_medium_sprite(10*k, 10*k, 0);
    // my_mutex_unlock(&my_mutex);
    // simple_medium_sprite(100, 100, 0);
    // k++;

    /*if (get_controller_status_key(2))
    {
      simple_medium_sprite(50,50,0);
    }
    if (get_controller_status_key(3))
    {
      simple_medium_sprite(90,90,0);
    }*/
    if (*CartridgeStatus & 0x1)
    {
      FunctionPtr Fun = (FunctionPtr)((*CartridgeStatus) & 0xFFFFFFFC);
      Fun();
    }
  }

  return 0;
}

extern char _heap_base[];
extern char _stack[];

char *_sbrk(int numbytes)
{
  static char *heap_ptr = NULL;
  char *base;

  if (heap_ptr == NULL)
  {
    heap_ptr = (char *)&_heap_base;
  }

  if ((heap_ptr + numbytes) <= _stack)
  {
    base = heap_ptr;
    heap_ptr += numbytes;
    return (base);
  }
  else
  {
    // errno = ENOMEM;
    return NULL;
  }
}
