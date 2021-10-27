#pragma once
#include <cstdint>
#include <array>
#include <tuple>
#include <vector>
#include <SDL.h>

class bus;

class ppu
{
private:
    std::uint16_t cycle;
    std::uint8_t mode;

    bus *gb_bus;

    std::vector<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>> sprite_array;
    std::array<std::uint8_t, 160> scanline;
    std::array<std::uint8_t, 160 * 144> frame;

public:
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    ppu();

    void connect_bus(bus *b);
    void clock();

    std::uint8_t read(std::uint16_t address);
    void write(std::uint16_t address, std::uint8_t data);
};