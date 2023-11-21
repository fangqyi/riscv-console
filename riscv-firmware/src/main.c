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
volatile int global = 42;

// int pthread_mutex_lock(pthread_mutex_t *mutex);
// int pthread_mutex_unlock(pthread_mutex_t *mutex);
// to here


//cartridge setup
volatile uint32_t *CartridgeStatus = (volatile uint32_t *)(0x4000001C);
typedef void (*FunctionPtr)(void);
void simple_medium_sprite(int16_t x, int16_t y, int16_t z);


int main(){
    //simple_medium_sprite(0,0,0);
    //loading cartridge
    while (1){
        if(*CartridgeStatus & 0x1){
            FunctionPtr Fun = (FunctionPtr)((*CartridgeStatus) & 0xFFFFFFFC);
            Fun();
        }
    }
    
    return 0;
}


extern char _heap_base[];
extern char _stack[];

char *_sbrk(int numbytes){
  static char *heap_ptr = NULL;
  char *base;

  if (heap_ptr == NULL) {
    heap_ptr = (char *)&_heap_base;
  }

  if((heap_ptr + numbytes) <=_stack) {
    base = heap_ptr;
    heap_ptr += numbytes;
    return (base);
  }
  else {
    //errno = ENOMEM;
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
