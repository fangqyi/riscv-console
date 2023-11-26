#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile uint32_t controller_status = 0;
volatile int global = 0;

// int pthread_mutex_lock(pthread_mutex_t *mutex);
// int pthread_mutex_unlock(pthread_mutex_t *mutex);
// to here

// cartridge setup
volatile uint32_t *CartridgeStatus = (volatile uint32_t *)(0x4000001C);
typedef void (*FunctionPtr)(void);
void simple_medium_sprite(uint16_t x, uint16_t y, uint16_t z);
void simple_display_text(char *new_text, uint32_t start_idx);
uint32_t get_controller_status_key(uint8_t key_idx);

int main()
{

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

// pthread_mutex_t locks[100];  // Assuming you have 100 locks

// void lock(int lockno) {
//     if (lockno < 0 || lockno >= 100) {
//         // Invalid lock number
//         return;
//     }

//     pthread_mutex_lock(&locks[lockno]);
// }

// void unlock(int lockno) {
//     if (lockno < 0 || lockno >= 100) {
//         // Invalid lock number
//         return;
//     }

//     pthread_mutex_unlock(&locks[lockno]);
// }
