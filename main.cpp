#include "chip8.h"
#include <iostream>
#include <SDL.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 640;

// The window we'll be rendering to
SDL_Window* gWindow = nullptr;

// The window renderer
SDL_Renderer* gRenderer = nullptr;

Chip8 myChip8;

bool setupGraphics();
void drawGraphics();

int main(int argc, char* args[]) {

    // Set up render system and register input callbacks
    setupGraphics();

    // Initialize the Chip8 system and load the game into the memory
    myChip8.initialize();

    // Event handler
    SDL_Event e;

    bool loadSucceeded = myChip8.loadProgram("TETRIS");
    if (!loadSucceeded) {
        printf("Program loading failed!");
        return 0;
    }

    // Emulation loop
    for(;;)
    {
        while(SDL_PollEvent(&e) != 0) {
            if(e.type == SDL_QUIT) {
                return 0;
            }
        }
        // Clear screen
        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x0011, 0x00, 0xFF );
        SDL_RenderClear( gRenderer );
        // Emulate one cycle
        myChip8.emulateCycle();

        // If the draw flag is set, update the screen
        if(myChip8.getDrawFlag()) {
            drawGraphics();
        }

        // Store key press state (Press and Release)
        // NOTE: Currently implemented as continuous-response keys
        myChip8.setKeys();
        SDL_RenderPresent( gRenderer );
        SDL_Delay( 2 );
    }
}

bool setupGraphics() {
    // Initialization flag
    bool success = true;

    // Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        success = false;
    }
    else
    {
        // Create window
        gWindow = SDL_CreateWindow( "Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == nullptr )
        {
            std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
            success = false;
        }
        else
        {
            // Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == nullptr )
            {
                std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << "\n";
                success = false;
            }
            else
            {
                // Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
            }

        }
    }

    return success;
}

void drawGraphics() {
    int scale = 20;
    const unsigned char* graphicsRaw = myChip8.getGraphics();
    for (unsigned short yLine = 0; yLine < 32; ++yLine) {
        for (unsigned short xLine = 0; xLine < 64; ++xLine) {
            if (graphicsRaw[xLine + yLine * 64] != 0) {
                auto x = xLine * scale;
                auto y = yLine * scale;
                SDL_Rect fillRect = { x, y, scale, scale};
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                SDL_RenderFillRect( gRenderer, &fillRect );
            } else {
                auto x = xLine * scale;
                auto y = yLine * scale;
                SDL_Rect fillRect = { x, y, scale, scale};
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
                SDL_RenderFillRect( gRenderer, &fillRect );
            }
        }
    }
}