//
// Created by tatu on 10.8.2018.
//

#include "chip8.h"

#include <iostream>
#include <ctime>

#include <SDL.h>

void Chip8::initialize() {
    pc = 0x200;     // PC starts at 0x200 on chip-8
    opcode = 0;     // Reset current opcode
    I = 0;          // Reset index register
    sp = 0;         // Reset stack pointer
    drawFlag = true;// Reset draw flag

    std::srand(std::time(nullptr));

    clearDisplay();
    clearStack();
    clearRegisters();
    clearMemory();
    loadFontset();

}

void Chip8::emulateCycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode opcode
    // First, look on the first nibble (4 bits)
    switch (opcode & 0xF000) {
        case 0x0000: // 0x00E0 or 0x00EE, display_clear or subroutine return
            // Check the latest nibble
            switch (opcode & 0x000F) {
                case 0x0000: // 0x00E0, display_clear()
                    clearDisplay();
                    break;
                case 0x000E: // 0x00EE, subroutine return
                {
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;
                }
                default:
                    std::cout << "Unknown opcode [0x0000]: 0x" << std::uppercase << std::hex << opcode << "\n" ;
            }
            break;
        case 0x1000: // 0x1NNN, Jump to NNN
        {
            pc = opcode & 0x0FFF;
            break;
        }
        case 0x2000: // 0x2NNN, Subroutine call at NNN
        {
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;
        }
        case 0x3000: // 0x3XNN, If (Vx==NN), skip the next instr. if VX==NN
        {
            if (V[(opcode & 0x0FFF) >> 8] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }
        case 0x4000: // 0x4XNN, if (Vx!=NN)
        {
            if (V[(opcode & 0x0FFF) >> 8] != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }
        case 0x5000: // 0x5XY0, skip next instruction if (Vx==Vy)
        {
            if (V[(opcode & 0x0FFF) >> 8] == V[((opcode & 0x00FF) >> 4)]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }
        case 0x6000: // 0x6XNN, sets Vx = NN
        {
            V[(opcode & 0x0FFF) >> 8] = (opcode & 0x00FF);
            pc += 2;
            break;
        }
        case 0x7000: // 0x7XNN, Vx += NN (carry flag not changed)
        {
            V[(opcode & 0x0FFF) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;
        }
        case 0x8000: // 0x8XY*, several different cases
        {
            switch (opcode & 0x000F) {
                case 0x0000:
                {
                    // Assign Vx to the value of Vy
                    V[(opcode & 0x0FFF) >> 8] = V[(opcode & 0x00FF) >> 4];
                    pc += 2;
                    break;
                }
                case 0x0001:
                {
                    // Sets Vx = Vx|Vy, bitwise OR
                    V[(opcode & 0x0FFF) >> 8] = (V[(opcode & 0x0FFF) >> 8] | V[(opcode & 0x00FF) >> 4]);
                    pc += 2;
                    break;
                }
                case 0x0002:
                {
                    // Sets Vx = Vx&Vy
                    V[(opcode & 0x0FFF) >> 8] = (V[(opcode & 0x0FFF) >> 8] & V[(opcode & 0x00FF) >> 4]);
                    pc += 2;
                    break;
                }
                case 0x0003:
                {
                    // Sets Vx = Vx^Vy, Vx to Vx xor Vy
                    V[(opcode & 0x0FFF) >> 8] = (V[(opcode & 0x0FFF) >> 8] ^ V[(opcode & 0x00FF) >> 4]);
                    pc += 2;
                    break;
                }
                case 0x0004:
                {
                    // Vx += Vy, VF is set to 1 when there's a carry, and to 0 when there isn't.
                    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00)])) {
                        V[0xF] = 1; // Set carry flag
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0FFF) >> 8] += V[(opcode & 0x00FF) >> 4];
                    pc += 2;
                    break;
                }
                case 0x0005:
                {
                    // Vx -= Vy, VF is set to 0 when there's a borrow, and 1 when there isn't.
                    if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
                        V[0xF] = 0; // Borrow occurred
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0FFF) >> 8] -= V[(opcode & 0x00FF) >> 4];
                    pc += 2;
                    break;
                }
                case 0x0006:
                {
                    // Vx >>=1, Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x1);
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;
                }
                case 0x0007:
                {
                    // Vx=Vy-Vx, VF is set to 0 when there's a borrow, and 1 when there isn't.
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
                        V[0xF] = 0; // Borrow occurred
                    } else {
                        V[0xF] = 1;
                    }
                    V[(opcode & 0x0FFF) >> 8] = V[(opcode & 0x00FF) >> 4] - V[(opcode & 0x0FFF) >> 8];
                    pc += 2;
                    break;
                }
                case 0x000E:
                {
                    // Vx<<=1, Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    V[0xF] = (V[(opcode & 0x0F00) >> 8] >> 7);
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;
                }
                default:
                    std::cout << "Unknown opcode: 0x" << std::uppercase << std::hex << opcode << "\n" ;
            }
            break;
        }
        case 0x9000: { // 0x9XY0, skips the next instruction if VX doesn't equal VY
            if (V[(opcode & 0x0FFF) >> 8] != V[(opcode & 0x00FF) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        }
        case 0xA000: { // 0xANNN, sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        }
        case 0xB000: { // 0xBNNN, jump to address NNN plus V0
            pc = (opcode & 0x0FFF) + V[0];
            pc += 2;
            break;
        }
        case 0xC000: { // 0xCXNN, Vx=rand()&NN
            V[(opcode & 0x0FFF) >> 8] = ((std::rand() % 256) & (opcode & 0x00FF));
            pc += 2;
            break;
        }
        case 0xD000: { // 0xDXYN, draw(Vx, Vy, N)
            unsigned short height;  // Amount of lines, width is 8px
            unsigned short x;       // Coordinate x
            unsigned short y;       // Coordinate y
            unsigned short pixel;

            height = (opcode & 0x000F);
            x = V[(opcode & 0x0F00) >> 8];
            y = V[(opcode & 0x00F0) >> 4];

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if((pixel & (0x80 >> xline)) != 0) {                    // If pixel to be drawn is 1
                        if(gfx[(x + xline + ((y + yline) * 64))] == 1) {    // If the pixel on screen is already 1
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;           // XOR mode drawing
                    }
                }
            }

            drawFlag = true;
            pc += 2;
            break;
        }
        case 0xE000: { // 0xEX9E or 0xEXA1
            switch (opcode & 0x000F) {
                case 0x000E: {
                    // Skips the next instruction if the key stored in VX is pressed.
                    // (Usually the next instruction is a jump to skip a code block)
                    if (key[V[(opcode & 0x0FFF) >> 8]] != 0) { // key pressed
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                }
                case 0x0001: {
                    // Skips the next instruction if the key stored in VX isn't pressed.
                    // (Usually the next instruction is a jump to skip a code block)
                    if (key[V[(opcode & 0x0FFF) >> 8]] == 0) { // key not pressed
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                }
                default: {
                    std::cout << "Unknown opcode: 0x" << std::uppercase << std::hex << opcode << "\n" ;
                    break;
                }
            }
            break;
        }
        case 0xF000: {
            int x;
            x = ((opcode & 0x0F00) >> 8);
            switch (opcode & 0x00FF) {
                case 0x0007: {
                    // Vx = get_delay()	Sets VX to the value of the delay timer.
                    V[x] = delay_timer;
                    pc += 2;
                    break;
                }
                case 0x000A: {
                    // Vx = get_key()	A key press is awaited, and then stored in VX.
                    // (Blocking Operation. All instruction halted until next key event)
                    V[x] = getKey();
                    pc += 2;
                    break;
                }
                case 0x0015: {
                    // delay_timer(Vx)	Sets the delay timer to VX.
                    delay_timer = V[x];
                    pc += 2;
                    break;
                }
                case 0x0018: {
                    // sound_timer(Vx)	Sets the sound timer to VX.
                    sound_timer += V[x];
                    pc += 2;
                    break;
                }
                case 0x001E: {
                    // I +=Vx	Adds VX to I.
                    I += V[x];
                    pc += 2;
                    break;
                }
                case 0x0029: {
                    // I=sprite_addr[Vx]	Sets I to the location of the sprite for the character in VX.
                    // Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    I = (V[x] * 0x5);
                    pc += 2;
                    break;
                }
                case 0x0033: {
                    // set_BCD(Vx);
                    //*(I+0)=BCD(3);
                    //
                    //*(I+1)=BCD(2);
                    //
                    //*(I+2)=BCD(1);
                    //
                    // Stores the binary-coded decimal representation of VX, with the most significant of three digits
                    // at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
                    // (In other words, take the decimal representation of VX, place the hundreds digit in memory at
                    // location in I, the tens digit at location I+1, and the ones digit at location I+2.)
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;
                }
                case 0x0055: {
                    // reg_dump(Vx,&I)	Stores V0 to VX (including VX) in memory starting at address I. The offset from
                    // I is increased by 1 for each value written, but I itself is left unmodified.
                    unsigned int I_it = I;
                    for (int it = 0; it <= x; ++it) {
                        memory[I_it] = V[it];
                        ++I_it;
                    }
                    pc += 2;
                    break;
                }
                case 0x0065: {
                    // reg_load(Vx,&I)	Fills V0 to VX (including VX) with values from memory starting at address I.
                    // The offset from I is increased by 1 for each value written, but I itself is left unmodified
                    unsigned int I_it = I;
                    for (int it = 0; it <= x; ++it) {
                        V[it] = memory[I_it];
                        ++I_it;
                    }
                    pc += 2;
                    break;
                }
            }
            break;
        }
        default: {
            std::cout << "Unknown opcode: 0x" << std::uppercase << std::hex << opcode << "\n" ;
        }
    }

    std::cout << "\nCurrent opcode: 0x" << std::uppercase << std::hex << opcode << "\n" ;

    // Update timers
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "Beep!\n";
            --sound_timer;
        }
    }


}

bool Chip8::loadProgram(std::string name) {
    // Start filling the memory from 0x200 = 512
    int memoryIndex = 512;
    const char * c_name;
    c_name = name.c_str();

    // Open the file in binary mode, with fopen
    FILE * programFile;
    programFile = fopen(c_name, "rb");

    // Read the program to the memory
    if (programFile == nullptr) {
        return false;
    } else {
        int c;
        while ((c = fgetc(programFile)) != EOF) {
            memory[memoryIndex] = c;
            ++memoryIndex;
            std::cout << "\nOpcode: " << std::uppercase << std::hex << c ;
        }
    }
    return true;
}

unsigned char Chip8::getKey() {
    while (true) {
        for (unsigned short i = 0; i < 16; ++i) {
            if (key[i] != 0) {
                return i;
            }
        }
    }
}

void Chip8::clearDisplay() {
    for (int i = 0; i < 2048; ++i) {
        gfx[i] = 0x0;
    }
    drawFlag = true;
}

void Chip8::clearStack() {
    for (int i = 0; i < 16; ++i) {
        stack[i] = 0x0;
    }
}

void Chip8::clearRegisters() {
    for (int i = 0; i < 16; ++i) {
        V[i] = 0x0;
    }
}

void Chip8::clearMemory() {
    for (int i = 0; i < 4096; ++i) {
        memory[i] = 0x0;
    }
}

void Chip8::loadFontset() {
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }
}

void Chip8::setKeys() {
    clearKeys();
    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
    if(currentKeyStates[SDL_SCANCODE_1]) {
        key[0] = 1;
    }
    if (currentKeyStates[SDL_SCANCODE_2]) {
        key[1] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_3]) {
        key[2] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_4]) {
        key[3] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_Q]) {
        key[4] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_W]) {
        key[5] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_E]) {
        key[6] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_R]) {
        key[7] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_A]) {
        key[8] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_S]) {
        key[9] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_D]) {
        key[10] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_F]) {
        key[11] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_Z]) {
        key[12] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_X]) {
        key[13] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_C]) {
        key[14] = 1;
    }
    if(currentKeyStates[SDL_SCANCODE_V]) {
        key[15] = 1;
    }
}

void Chip8::clearKeys() {
    for (int i = 0; i < 16; ++i) {
        key[i] = 0;
    }
}

bool Chip8::getDrawFlag() {
    return drawFlag;
}

const unsigned char *Chip8::getGraphics() {
    return gfx;
}
