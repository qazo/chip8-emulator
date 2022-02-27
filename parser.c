#include "headers/parser.h"


inline uint16_t parse_opcode(uint8_t byte_one, uint8_t byte_two){
    return ((uint16_t)byte_one << 8) | byte_two;
}

void start_game_loop(chip8_hardware_t* hardware){
    printf("[+] Starting game\n");
    // Initialize random number generator
    srand(time(NULL));
    hardware->registers->PC = GAMESTART;
    SDL_Event event;

    SDL_AudioSpec wavSpec;
    uint32_t wavLength;
    uint8_t* wavBuffer;

    SDL_LoadWAV("./sound/beep-02.wav", &wavSpec, &wavBuffer, &wavLength);

    // Open audio device
    SDL_AudioDeviceID deviceID = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    for(int i = 0; i < 64; i++){
        for(int j = 0; j < 32; j++) {
            hardware->screen[i][j] = 0;
        }
    }

    uint8_t keymap[16] = {
        SDLK_x,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_q,
        SDLK_w,
        SDLK_e,
        SDLK_a,
        SDLK_s,
        SDLK_d,
        SDLK_z,
        SDLK_c,
        SDLK_4,
        SDLK_r,
        SDLK_f,
        SDLK_v
    };

    SDL_Window* window = SDL_CreateWindow("Chip8-Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 896, 448, SDL_RENDERER_ACCELERATED);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, NOFLAGS);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderSetLogicalSize(renderer, 64, 32);
    SDL_RenderClear(renderer);
    while(1) {
    uint64_t start = SDL_GetTicks();

    while(SDL_PollEvent(&event)){
        switch(event.type) {
            case SDL_QUIT:
                exit(1);
                break;
            case SDL_KEYDOWN:
                for(int i = 1; i < 16; i++) {
                    if(event.key.keysym.sym == keymap[i]){
                        hardware->keyboard[i] = TRUE;
                    }
                }
                break;
            case SDL_KEYUP:
              for(int i = 1; i < 16; i++) {
                    if(event.key.keysym.sym == keymap[i]){
                        hardware->keyboard[i] = FALSE;
                    }
                }
                break;
        }
    }

    for(int i = 0; i < 1000; i++){
        uint16_t opcode = parse_opcode(hardware->main_memory[hardware->registers->PC], hardware->main_memory[hardware->registers->PC + 1]);
        execute_opcode(opcode, hardware);
    }

        if(hardware->registers->DT > 0) {
            hardware->registers->DT--;
        }

        if(hardware->registers->ST > 0) {
            SDL_QueueAudio(deviceID, wavBuffer, wavLength);
            SDL_PauseAudioDevice(deviceID, UNPAUSE);
            hardware->registers->ST--;
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for(int i = 0; i < 64; i++){
            for(int j = 0; j < 32; j++){
                if(hardware->screen[i][j] == 1){
                    SDL_RenderDrawPoint(renderer, i , j);
                }
            }
        }
        SDL_RenderPresent(renderer);

        uint64_t end = SDL_GetTicks();

        float timeElapsed = (end-start)/(float)SDL_GetPerformanceFrequency() * 1000.0f;
       // SDL_Delay(floor(16.666f - timeElapsed));

    }
}

uint8_t rand_byte(){
    int random_num = rand() % 255 + 0;
    return (uint8_t)random_num;
}

void execute_opcode(uint16_t opcode, chip8_hardware_t* hardware) {
    // Switch statment here to handle all opcodes
    switch ((opcode & 0xF000) >> 12) {
    case 0x0:
        if((opcode & 0x000F) > 0){
            printf("Executed 0xE\n");
            hardware->registers->PC = pop(hardware->stack);
        } else {
               printf("Executed 0x0\n");
         for(int i = 0; i < 64; i++){
            for(int j = 0; j < 32; j++) {
                hardware->screen[i][j] = 0;
            }
          }
        }
        hardware->registers->PC = hardware->registers->PC + 2;
        break;
    case 0x1:
       printf("Executed 0x1\n");
        hardware->registers->PC = VALNNN;
        break;
    case 0x2:
    printf("Executed 0x2\n");
        push(hardware->stack, hardware->registers->PC);
        hardware->registers->PC = VALNNN;
        break;
    case 0x3:
    printf("Executed 0x3\n");
        if(hardware->registers->V[REGISTERX] == VALNN) {
            hardware->registers->PC = hardware->registers->PC + 4;
        } else {
            hardware->registers->PC = hardware->registers->PC + 2;
        }
        break;
    case 0x4:
    printf("Executed 0x4\n");
        if(hardware->registers->V[REGISTERX] != VALNN){
            hardware->registers->PC = hardware->registers->PC + 4;
        } else {
            hardware->registers->PC = hardware->registers->PC + 2;
        }
        break;
    case 0x5:
    printf("Executed 0x5\n");
        if(hardware->registers->V[REGISTERX] == hardware->registers->V[REGISTERY]){
            hardware->registers->PC = hardware->registers->PC + 4;
        } else {
            hardware->registers->PC = hardware->registers->PC + 2;
        }
        break;
    case 0x6:
    printf("Executed 0x6\n");
        hardware->registers->V[REGISTERX] = VALNN;
        hardware->registers->PC = hardware->registers->PC + 2;
        break;
    case 0x7:
    printf("Executed 0x7\n");
        hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] + VALNN;
        hardware->registers->PC = hardware->registers->PC + 2;
        break;
    case 0x8:
        switch(opcode & 0x000F) {
            case 0x0:
            printf("Executed 0x80\n");
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERY];
                break;
            case 0x1:
             printf("Executed 0x81\n");
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] | hardware->registers->V[REGISTERY];
                break;
            case 0x2:
             printf("Executed 0x82\n");
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] & hardware->registers->V[REGISTERY];
                break;
            case 0x3:
             printf("Executed 0x83\n");
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] ^ hardware->registers->V[REGISTERY];
                break;
            case 0x4:
             printf("Executed 0x84\n");
                if((hardware->registers->V[REGISTERX] + hardware->registers->V[REGISTERY]) > 0xFF) {
                    hardware->registers->V[REGISTERF] = 1;
                } else {
                    hardware->registers->V[REGISTERF] = 0;
                }
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] + hardware->registers->V[REGISTERY];
                break;
            case 0x5:
             printf("Executed 0x85\n");
                if(hardware->registers->V[REGISTERX] > hardware->registers->V[REGISTERY]) {
                    hardware->registers->V[REGISTERF] = 1;
                } else {
                    hardware->registers->V[REGISTERF] = 0;
                }
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] - hardware->registers->V[REGISTERY];
                break;
            case 0x6:
             printf("Executed 0x86\n");
                hardware->registers->V[REGISTERF] = hardware->registers->V[REGISTERX] & 0x01;
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] >> 1;
                break;
            case 0x7:
             printf("Executed 0x87\n");
                if(hardware->registers->V[REGISTERY] >= hardware->registers->V[REGISTERX]) {
                    hardware->registers->V[REGISTERF] = 1;
                } else {
                    hardware->registers->V[REGISTERF] = 0;
                }

                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERY] - hardware->registers->V[REGISTERX];
                break;
            case 0xE:
             printf("Executed 0x8E\n");
                hardware->registers->V[REGISTERF] = hardware->registers->V[REGISTERX] >> 7;
                hardware->registers->V[REGISTERX] = hardware->registers->V[REGISTERX] << 1;
                break;
        }
        hardware->registers->PC = hardware->registers->PC + 2;
        break;
    
    case 0x9:
     printf("Executed 0x9\n");
        if(hardware->registers->V[REGISTERX] != hardware->registers->V[REGISTERY]) {
            hardware->registers->PC = hardware->registers->PC + 4;
        } else {
            hardware->registers->PC = hardware->registers->PC + 2;
        }
        break;
    case 0xA:
    printf("Executed 0xA\n");
        hardware->registers->I = VALNNN;
        hardware->registers->PC = hardware->registers->PC + 2;
        break;
    case 0xB:
    printf("Executed 0xB\n");
        hardware->registers->PC = hardware->registers->V[0] + VALNNN;
        break;
    case 0xC:
     printf("Executed 0xC\n");
        hardware->registers->V[REGISTERX] = rand_byte() & VALNN;
        hardware->registers->PC = hardware->registers->PC + 2;
    case 0xD:
     printf("Executed 0xD\n");
        // Draw Instruction: DXYN
        hardware->registers->V[REGISTERF] = 0;
        // Loop over N bytes
        for(int i = 0; i < (opcode & 0x000F); i++){
            uint8_t xcord = hardware->registers->V[REGISTERX];
            uint8_t ycord = hardware->registers->V[REGISTERY];
            uint8_t byte = hardware->main_memory[hardware->registers->I + i];
            for(int j = 0; j < 8; j++) {
                uint8_t bit = (byte << j) & 0x80;
                if(bit != 0){
                    if(xcord + j > 63){
                        continue;
                    } else if(ycord + i > 31) {
                        continue;
                    } else {
                        if(hardware->screen[xcord + j][ycord + i] == 1) {
                            // Set collision flag
                            hardware->registers->V[REGISTERF] = 1;
                        }
                        hardware->screen[xcord + j][ycord + i] ^= 1;
                    }
                }
            }
        }
        hardware->registers->PC = hardware->registers->PC + 2;

        break;
    case 0xE:
        switch(opcode & 0x000F) {
            case 0xE:
             printf("Executed 0xEE\n");
                if(hardware->keyboard[hardware->registers->V[REGISTERX]] != FALSE){
                    hardware->registers->PC = hardware->registers->PC + 4;
                } else {
                    hardware->registers->PC = hardware->registers->PC + 2;
                }
                break;
            case 0x1:
             printf("Executed 0xE1\n");
                if(hardware->keyboard[hardware->registers->V[REGISTERX]] == FALSE){
                    hardware->registers->PC = hardware->registers->PC + 4;
                } else {
                    hardware->registers->PC = hardware->registers->PC + 2;
                }
                break;
        }
        break;
    case 0xF:
        switch(opcode & 0x00FF){
            case 0x07:
             printf("Executed 0xF7\n");
                hardware->registers->V[REGISTERX] = hardware->registers->DT;
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
            case 0x0A:
            printf("Executed 0xFA\n");
                for(int i = 0; i < 16; i++){
                    if(hardware->keyboard[i] == TRUE) {
                        hardware->registers->V[REGISTERX] = i;
                        hardware->registers->PC = hardware->registers->PC + 2;
                        break;
                    }
                }
                break;
            case 0x15:
            printf("Executed 0xF15\n");
                hardware->registers->DT = hardware->registers->V[REGISTERX];
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
            case 0x18:
            printf("Executed 0xF18\n");
                hardware->registers->ST = hardware->registers->V[REGISTERX];
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
            case 0x1E:
            printf("Executed 0xF1E\n");
                hardware->registers->I = hardware->registers->I + hardware->registers->V[REGISTERX];
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
            case 0x29:
            printf("Executed 0xF29\n");
                hardware->registers->I = hardware->registers->V[REGISTERX] * 0x05;
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
            case 0x33:
            printf("Executed 0xF33\n");
                // set hundreds place value in memory.
                hardware->main_memory[hardware->registers->I] = hardware->registers->V[REGISTERX] / 100;
                // set tens place value in memory.
                hardware->main_memory[hardware->registers->I + 1] = (hardware->registers->V[REGISTERX] / 10) % 10;
                // set ones place value in memory.
                hardware->main_memory[hardware->registers->I + 2] = hardware->registers->V[REGISTERX] % 10;
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
            case 0x55:
            printf("Executed 0xF55\n");
                for(int reg = 0; reg <= REGISTERX; reg++) {
                    hardware->main_memory[hardware->registers->I + reg] = hardware->registers->V[reg];
                } 
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
            case 0x65:
            printf("Executed 0x65\n");
                for(int reg = 0; reg <= REGISTERX; reg++){
                    hardware->registers->V[reg] = hardware->main_memory[hardware->registers->I + reg];
                } 
                hardware->registers->PC = hardware->registers->PC + 2;
                break;
        }
        break;
    default:
        break;
    }
}

