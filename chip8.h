#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include <string>

class Chip8 {
public:
    // Constructor.
    Chip8() = default;

    // Initializer.
    void initialize();

    // Emulates one cpu cycle.
    void emulateCycle();

    // Load the program to memory. Returns false if load failed.
    bool loadProgram(std::string name);

    // Waits until a key is pressed and then returns the pressed key.
    // Blocks all other operations until received a key press
    unsigned char getKey();

    // Clears display, sets all gfx[] to zero
    void clearDisplay();

    // Clears stack
    void clearStack();

    // Clear registers
    void clearRegisters();

    // Clear memory
    void clearMemory();

    // Loads the fontset to memory
    void loadFontset();

    // Set currently pressed keys
    void setKeys();

    // Clear the pressed keys array
    void clearKeys();

    // Returns the draw flag
    bool getDrawFlag();

    // Returns the screen pointer
    const unsigned char* getGraphics();

private:
    unsigned short opcode;          // For storing the current opcode.
    unsigned char memory[4096];     // Emulated total memory of 4K bytes.
    unsigned char V[16];            // Emulated CPU registers.
    unsigned short I;               // Index register.
    unsigned short pc;              // Program counter.
    unsigned char gfx[64 * 32];     // Black and white graphics screen pixels.
    unsigned char delay_timer;      // Delay timer. Count at 60hz, or zero, if set above zero.
    unsigned char sound_timer;      // Sound timer. System buzzer sounds when the timer reaches zero.
    unsigned short stack[16];       // Jump call stack.
    unsigned short sp;              // Stack pointer.
    unsigned char key[16];          // Current state of the hex keypad. 1 = pressed, 0 = released
    bool drawFlag;                  // If set true, need to redraw the screen

    unsigned char chip8_fontset[80] =
            {
                    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                    0x20, 0x60, 0x20, 0x20, 0x70, // 1
                    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
            };
};


#endif //CHIP8_CHIP8_H
