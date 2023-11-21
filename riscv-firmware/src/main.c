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


// added part.
typedef void (*TThreadEntry)(void *);
typedef uint32_t *TThreadContext;

void OtherThreadFunction(void *);

// this can be thought as fork() function
TThreadContext InitThread(uint32_t *stacktop, TThreadEntry entry,void *param);

// 
void SwitchThread(TThreadContext *oldcontext, TThreadContext newcontext);

TThreadContext OtherThread;
TThreadContext MainThread;
// to here


//cartridge setup
volatile uint32_t *CartridgeStatus = (volatile uint32_t *)(0x4000001C);
typedef void (*FunctionPtr)(void);
void simple_medium_sprite(int16_t x, int16_t y, int16_t z);

int main(){
    uint32_t OtherThreadStack[128];

    //loading cartridge
    while (1){
        simple_medium_sprite_test_key(0,0,0);
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

void OtherThreadFunction(void *){
    int last_global = global;
    while(1){
        if(global != last_global){
            SwitchThread(&OtherThread,MainThread);
            last_global = global;
        }
    }
}




/* ---------- normal functions ---------- */
/*
pid_t fork(void) {
    pid_t child_pid = -1;
    child_pid = fork();
    
    if (child_pid < 0) {
        // Error occurred
        if (errno == EAGAIN) {
            // Error code -1 for exceeding the limit
            return -1;
        } else {
            // Other error
            return -2;
        }
    }
    
    return child_pid;
}

pthread_mutex_t locks[100];  // Assuming you have 100 locks

void lock(int lockno) {
    if (lockno < 0 || lockno >= 100) {
        // Invalid lock number
        return;
    }
    
    pthread_mutex_lock(&locks[lockno]);
}


void unlock(int lockno) {
    if (lockno < 0 || lockno >= 100) {
        // Invalid lock number
        return;
    }
    
    pthread_mutex_unlock(&locks[lockno]);
}

pid_t wait(int* status) {
    pid_t terminated_child_pid;
    int child_status;
    
    terminated_child_pid = wait(&child_status);
    
    if (terminated_child_pid < 0) {
        // Error occurred
        return -1;
    }
    
    if (WIFEXITED(child_status)) {
        // Child terminated normally
        *status = WEXITSTATUS(child_status);
    } else {
        // Child didn't terminate normally
        *status = -1;  // You may need to define error codes
    }
    
    return terminated_child_pid;
}
*/