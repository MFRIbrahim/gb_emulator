#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <SDL.h>

#include "cpu.h"
#include "bus.h"
#include "timer.h"
#include "ppu.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "A ROM file is required!" << std::endl;
        return 1;
    }

    cpu gb_cpu = cpu();
    ppu gb_ppu = ppu();
    bus gb_bus = bus();
    timer gb_timer = timer();
    gb_bus.load_rom(argv[1]);
    bus *b = &gb_bus;
    gb_cpu.connect_bus(b);
    gb_ppu.connect_bus(b);
    gb_timer.connect_bus(b);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("gb_emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          160 * 2, 144 * 2, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                             160, 144);
    gb_ppu.window = window;
    gb_ppu.renderer = renderer;
    gb_ppu.texture = texture;

    std::uint32_t control_counter = 0;
    bool quit = false;
    while (!quit)
    {
        ++control_counter;
        if (control_counter == 69905)
        {
            control_counter = 0;
            const Uint8 *key_states = SDL_GetKeyboardState(NULL);

            if (key_states[SDL_SCANCODE_RIGHT])
            {
                gb_bus.set_direction_button(0, 1);
            }
            if (key_states[SDL_SCANCODE_LEFT])
            {
                gb_bus.set_direction_button(1, 1);
            }
            if (key_states[SDL_SCANCODE_UP])
            {
                gb_bus.set_direction_button(2, 1);
            }
            if (key_states[SDL_SCANCODE_DOWN])
            {
                gb_bus.set_direction_button(3, 1);
            }
            if (!key_states[SDL_SCANCODE_RIGHT])
            {
                gb_bus.set_direction_button(0, 0);
            }
            if (!key_states[SDL_SCANCODE_LEFT])
            {
                gb_bus.set_direction_button(1, 0);
            }
            if (!key_states[SDL_SCANCODE_UP])
            {
                gb_bus.set_direction_button(2, 0);
            }
            if (!key_states[SDL_SCANCODE_DOWN])
            {
                gb_bus.set_direction_button(3, 0);
            }

            if (key_states[SDL_SCANCODE_S])
            {
                gb_bus.set_action_button(0, 1);
            }
            if (key_states[SDL_SCANCODE_A])
            {
                gb_bus.set_action_button(1, 1);
            }
            if (key_states[SDL_SCANCODE_Y])
            {
                gb_bus.set_action_button(2, 1);
            }
            if (key_states[SDL_SCANCODE_X])
            {
                gb_bus.set_action_button(3, 1);
            }
            if (!key_states[SDL_SCANCODE_S])
            {
                gb_bus.set_action_button(0, 0);
            }
            if (!key_states[SDL_SCANCODE_A])
            {
                gb_bus.set_action_button(1, 0);
            }
            if (!key_states[SDL_SCANCODE_Y])
            {
                gb_bus.set_action_button(2, 0);
            }
            if (!key_states[SDL_SCANCODE_X])
            {
                gb_bus.set_action_button(3, 0);
            }

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    quit = true;
                    break;
                }
            }
        }

        gb_cpu.handle_interrupt();
        gb_ppu.clock();
        gb_timer.clock();
        gb_bus.dma_clock();
        gb_cpu.clock();
    }

    SDL_Quit();

    return 0;
}