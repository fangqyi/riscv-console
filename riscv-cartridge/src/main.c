#include <stdint.h>

// Define TEXT_MODE and GRAPHICS_MODE
#define TEXT_MODE        0x0
#define GRAPHICS_MODE    0x1

/* Graphics parameters */
#define WIDTH 0x40 // Grid width of 0x40
#define HEIGHT 0x23 // Grid height of 0x23
#define SPRITE_DATA_SIZE  256 // Size of sprite data in bytes
#define CONTROL_SIZE      4   // Size of control data in bytes
#define TOTAL_SPRITE_SIZE (SPRITE_DATA_SIZE + CONTROL_SIZE) // Total size of sprite and control data
#define PALETTE_SIZE 256

/* Graphics function */
uint32_t CalculatePremultipliedRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  if (a != 255) {
    r = (r * a) / 255;
    g = (g * a) / 255;
    b = (b * a) / 255;
  }
  return (a << 24) | (b << 16) | (g << 8) | r;
}

volatile int global = 42;
volatile uint32_t controller_status = 0;

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xF4800);
int main() {
    int a = 4;
    int b = 12;
    int last_global = 42;
    int x_pos = 12;
    int countdown =1;

    /* Graphics parameters */
    uint16_t Grapics_X = 50;
    uint16_t Grapics_Y = 50;
    uint16_t Grapics_Z = 3;
    uint8_t* sprite_data = (uint8_t*)malloc(TOTAL_SPRITE_SIZE * sizeof(uint8_t));
    uint8_t* clear_data = (uint8_t*)malloc(TOTAL_SPRITE_SIZE * sizeof(uint8_t));
    uint32_t* palette_data = (uint32_t*)malloc(PALETTE_SIZE * sizeof(uint32_t));
    //
    unit_32t MODE_CONTROL;

    VIDEO_MEMORY[0] = 'H';
    VIDEO_MEMORY[1] = 'e';
    VIDEO_MEMORY[2] = 'l';
    VIDEO_MEMORY[3] = 'l';
    VIDEO_MEMORY[4] = 'o';
    VIDEO_MEMORY[5] = ' ';
    VIDEO_MEMORY[6] = 'W';
    VIDEO_MEMORY[7] = 'o';
    VIDEO_MEMORY[8] = 'r';
    VIDEO_MEMORY[9] = 'l';
    VIDEO_MEMORY[10] = 'd';
    VIDEO_MEMORY[11] = '!';
    VIDEO_MEMORY[12] = 'X';

    //  API Calls Parameters
    //uint32_t SetBackground_API_Parameters[] = {6, };

  while (1) {
        int c = a + b + global;
        if(global != last_global){
          /*
            if(controller_status){
                VIDEO_MEMORY[x_pos] = ' ';
                if(controller_status & 0x1){
                    if(x_pos & 0x3F){
                        x_pos--;
                    }
                }
                if(controller_status & 0x2){
                    if(x_pos >= 0x40){
                        x_pos -= 0x40;
                    }
                }
                if(controller_status & 0x4){
                    if(x_pos < 0x8C0){
                        x_pos += 0x40;
                    }
                }
                if(controller_status & 0x8){
                    if((x_pos & 0x3F) != 0x3F){
                        x_pos++;
                    }
                }
                VIDEO_MEMORY[x_pos] = 'X';
            }
            */
          if(controller_status){
            uint32_t syscall_params_mode[1];
            syscall_params_mode[0] = GET_MODE_CONTROL_REGISTER; // GET_MODE_CONTROL_REGISTER syscall number
            MODE_CONTROL = c_syscall(syscall_params_mode);

            if(MODE_CONTROL == GRAPHICSMODE){
              uint8_t sprite_idx; // 0~255
              uint8_t sprite_ctrl_idx = 0; // 0~255
              uint8_t sprite_data_idx = 0; // 0~255
              uint8_t palette_idx = 0; // 0~3

              uint32_t ClearX_API_Parameters[] = {7,X_Position};
              char clear_X[] = " ";
              SystemCall2(ClearX_API_Parameters,clear_X);

              uint32_t SmallSprite2_API_Parameters[] = {8, sprite_idx, spriteData ,sprite_ctrl_idx,sprite_data_idx,Grapics_X,Grapics_Y,Grapics_Z,palette_idx,palette_data};
              SystemCall(SmallSprite2_API_Parameters); // Draws a Small Sprite

              ControllerStatus = SystemCall(Controller_API_Parameters);  // "CONTROLLLER" status is fetched by passing '3' as parameter to the SystemCall()
              InterruptPendingReg = SystemCall(InterruptPending_API_Parameters); // "Interrupt Pending Regiser" status is fetched here !

              if(ControllerStatus){
                // Clear original sprite
                uint32_t SmallSprite2_API_Parameters[] = {8, sprite_idx, Clear_data ,sprite_ctrl_idx,sprite_data_idx,Grapics_X,Grapics_Y,Grapics_Z,palette_idx,palette_data};
                SystemCall(SmallSprite2_API_Parameters);

                if(ControllerStatus & 0x1){ // 'd' -> LEFT
                  if(Grapics_X > 0){
                    Grapics_X--;
                  }
                }
                if(ControllerStatus & 0x2){ // 'w' -> UP
                  if(Grapics_Y > 0){
                    Grapics_Y--;
                  }
                }
                if(ControllerStatus & 0x4){ // 'x' -> DOWN
                  if(Grapics_Y < (HEIGHT - 1)){
                    Grapics_Y++;
                  }
                }
                if(ControllerStatus & 0x8){ // 'a' -> RIGHT
                  if(Grapics_X < (WIDTH - 1)){
                    Grapics_X++;
                  }
                }

                uint32_t SmallSprite2_API_Parameters[] = {8, sprite_idx, data ,sprite_ctrl_idx,sprite_data_idx,Grapics_X,Grapics_Y,Grapics_Z,palette_idx,palette_data};
                SystemCall(SmallSprite2_API_Parameters);
              }

              if(MODE_CONTROL == TEXT_MODE){
              VIDEO_MEMORY[x_pos] = ' ';
              if(controller_status & 0x1){
                if(x_pos & 0x3F){
                  x_pos--;
                }
              }
              if(controller_status & 0x2){
                if(x_pos >= 0x40){
                  x_pos -= 0x40;
                }
              }
              if(controller_status & 0x4){
                if(x_pos < 0x8C0){
                  x_pos += 0x40;
                }
              }
              if(controller_status & 0x8){
                if((x_pos & 0x3F) != 0x3F){
                  x_pos++;
                }
              }
              VIDEO_MEMORY[x_pos] = 'X';
            }
          }
            last_global = global;
        }
        countdown--;
        if(!countdown){
            global++;
            controller_status = (*((volatile uint32_t *)0x40000018));
            countdown = 100000;

            // Display the video clock period
            display_video_clock_period();

            // Switch between text and graphics mode based on global value
            uint32_t syscall_params_switch[2];
            //int32_t syscall_params_mode[1];
            syscall_params_mode[0] = 2; // GET_MODE_CONTROL_REGISTER syscall number
            syscall_params_switch[0] = 5; // SWITCH_MODE syscall number
            syscall_params_switch[1] = (global % 2) ? 0x1 : 0x0; // Alternate between graphics and text mode
            c_syscall(syscall_params_switch); // Switch mode
            // c_syscall(syscall_params, NULL); // Switch mode
            MODE_CONTROL = c_syscall(syscall_params_mode); // Get the mode status
        }

    }

    return 0;
}
