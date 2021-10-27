#include <cstdint>
#include <algorithm>
#include <SDL.h>
#include <iostream>
#include <bitset>

#include "ppu.h"
#include "bus.h"
#include <chrono>
#include <thread>
#include <iomanip>

ppu::ppu()
{
    cycle = 0;
    mode = 2;

    scanline.fill(0);
    frame.fill(0);
}

std::uint8_t ppu::read(std::uint16_t address)
{
    return gb_bus->read(address);
}

void ppu::write(std::uint16_t address, std::uint8_t data)
{
    return gb_bus->write(address, data);
}

void ppu::connect_bus(bus *b)
{
    gb_bus = b;
}

void ppu::clock()
{
    if (cycle == 0)
    {
        std::uint8_t stat = read(0xff41);
        if (read(0xff44) == read(0xff45))
        {
            stat = stat | (1 << 2);
            write(0xff41, stat);
            if (stat & (1 << 6))
            {
                write(0xff0f, read(0xff0f) | (1 << 1));
            }
        }
        else
        {
            stat = stat & ~(1 << 2);
            write(0xff41, stat);
        }
        if (mode == 0)
        {
            if (stat & (1 << 3))
            {
                write(0xff0f, read(0xff0f) | (1 << 1));
            }
            stat = stat & ~(0b11 << 0);
            write(0xff41, stat);
            std::uint8_t ly_register = read(0xff44);
            write(0xff44, ++ly_register);
            cycle += 204;
            if (ly_register == 144)
            {
                mode = 1;
            }
            else
            {
                mode = 2;
            }
        }

        else if (mode == 1)
        {
            std::array<uint32_t, 4> color_map = {0xffffffff, 0xffc0c0c0, 0xff606060, 0xff000000};
            write(0xff0f, read(0xff0f) | (1 << 0));
            if (stat & (1 << 4))
            {
                write(0xff0f, read(0xff0f) | (1 << 1));
            }
            stat = stat & ~(1 << 1);
            stat = stat | (1 << 0);
            write(0xff41, stat);
            std::array<uint32_t, 160 * 144> sdl_buffer;
            for (size_t i = 0; i < frame.size(); ++i)
            {
                sdl_buffer[i] = color_map[frame[i]];
            }
            SDL_UpdateTexture(texture, NULL, &sdl_buffer, 160 * 4);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            frame.fill(0);
            cycle += 4560;
        }

        else if (mode == 2)
        {
            if (stat & (1 << 5))
            {
                write(0xff0f, read(0xff0f) | (1 << 1));
            }
            stat = stat & ~(1 << 0);
            stat = stat | (1 << 1);
            write(0xff41, stat);
            std::uint8_t ly = read(0xff44);
            std::uint8_t sprite_height = 8;
            bool obj_size = read(0xff40) & (1 << 2);
            if (obj_size)
            {
                sprite_height = 16;
            }
            std::uint8_t sprite_counter = 0;

            for (std::uint16_t address = 0xfe00; address < 0xfea0; address += 4)
            {
                std::uint8_t y_pos = read(address);
                std::uint8_t x_pos = read(address + 1);
                std::uint8_t tile_index = read(address + 2);
                std::uint8_t flags = read(address + 3);

                if ((ly + 16) >= y_pos && (ly + 16) < y_pos + sprite_height)
                {
                    if (sprite_counter < 10 && x_pos > 0 && x_pos < 168)
                    {
                        sprite_array.push_back({y_pos, x_pos, tile_index, flags});
                    }
                    sprite_counter += 1;
                }
            }
            std::reverse(sprite_array.begin(), sprite_array.end());
            std::stable_sort(sprite_array.begin(), sprite_array.end(), [](auto &left, auto &right)
                             { return std::get<1>(left) > std::get<1>(right); });
            cycle += 80;
            mode = 3;
        }

        else if (mode == 3)
        {
            stat = stat | (0b11 << 0);
            write(0xff41, stat);
            std::uint8_t lcdc = read(0xff40);
            std::uint8_t scy = read(0xff42);
            std::uint8_t scx = read(0xff43);
            std::uint8_t wy = read(0xff4a);
            std::uint8_t wx = read(0xff4b);
            std::uint8_t ly = read(0xff44);

            std::uint16_t tilemap_address = 0x9800;
            std::array<uint8_t, 160> scanline_color_ids;
            scanline_color_ids.fill(0);
            for (std::uint8_t x_coordinate = 0; x_coordinate < 160; x_coordinate += 8)
            {
                bool window_tile = (x_coordinate + 7) >= wx && ly >= wy && (lcdc & (1 << 5));
                if ((lcdc & (1 << 3) && !window_tile) || (lcdc & (1 << 6) && window_tile))
                {
                    tilemap_address = 0x9c00;
                }
                std::uint8_t tile_x = ((scx + x_coordinate) / 8) % 32;
                std::uint8_t tile_y = ((scy + ly) / 8) % 32;
                if (window_tile)
                {
                    std::uint8_t tile_x = (x_coordinate / 8) % 32;
                    std::uint8_t tile_y = (ly / 8) % 32;
                }
                std::uint8_t tile_index = read(tilemap_address + (tile_y * 32 + tile_x));
                std::uint16_t bg_window_tile_data_area = 0x9000;
                if (!(lcdc & (1 << 4)) && tile_index >= 128)
                {
                    bg_window_tile_data_area = 0x8800;
                    tile_index -= 128;
                }
                if (lcdc & (1 << 4))
                {
                    bg_window_tile_data_area = 0x8000;
                }
                std::uint16_t pixel_address = bg_window_tile_data_area + tile_index * 16 + ((scy + ly) % 8) * 2;
                std::uint8_t tile_low = read(pixel_address);
                std::uint8_t tile_high = read(pixel_address + 1);
                std::uint8_t bgp = read(0xff47);
                std::array<std::uint8_t, 4> color_ids = {(bgp & (0b11 << 0)) >> 0, (bgp & (0b11 << 2)) >> 2, (bgp & (0b11 << 4)) >> 4, (bgp & (0b11 << 6)) >> 6};
                for (int i = 7; i >= 0; --i)
                {
                    std::uint8_t color_value = ((tile_high & (1 << 0)) << 1) | ((tile_low & (1 << 0)) >> 0);
                    if (i != 0)
                    {
                        color_value = ((tile_high & (1 << i)) >> (i - 1)) | ((tile_low & (1 << i)) >> i);
                    }
                    if (!(lcdc & (1 << 0)))
                    {
                        scanline[x_coordinate + (7 - i)] = 0;
                        scanline_color_ids[x_coordinate + (7 - i)] = 0;
                    }
                    else
                    {
                        scanline[x_coordinate + (7 - i)] = color_ids[color_value];
                        scanline_color_ids[x_coordinate + (7 - i)] = color_value;
                    }
                }
            }

            std::uint8_t sprite_height = 8;
            bool obj_size = read(0xff40) & (1 << 2);
            if (obj_size)
            {
                sprite_height = 16;
            }
            for (std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t> obj : sprite_array)
            {
                auto [y_pos, x_pos, tile_index, flags] = obj;
                bool palette_number = flags & (1 << 4);
                bool x_flip = flags & (1 << 5);
                bool y_flip = flags & (1 << 6);
                bool bg_window_over_obj = flags & (1 << 7);
                std::uint16_t pixel_address = 0x8000 + tile_index * 16 + ((ly + 16) - y_pos) * 2;
                std::uint8_t tile_low = read(pixel_address);
                std::uint8_t tile_high = read(pixel_address + 1);
                if (y_flip)
                {
                    pixel_address = 0x8000 + tile_index * 16 + ((sprite_height - 1) - ((ly + 16) - y_pos)) * 2;
                    tile_low = read(pixel_address);
                    tile_high = read(pixel_address + 1);
                }
                std::uint8_t obp0 = read(0xff48);
                std::uint8_t obp1 = read(0xff49);
                std::array<std::uint8_t, 4> color_ids;
                if (palette_number == 0)
                {
                    color_ids[0] = (obp0 & (0b11 << 0)) >> 0;
                    color_ids[1] = (obp0 & (0b11 << 2)) >> 2;
                    color_ids[2] = (obp0 & (0b11 << 4)) >> 4;
                    color_ids[3] = (obp0 & (0b11 << 6)) >> 6;
                }
                else
                {
                    color_ids[0] = (obp1 & (0b11 << 0)) >> 0;
                    color_ids[1] = (obp1 & (0b11 << 2)) >> 2;
                    color_ids[2] = (obp1 & (0b11 << 4)) >> 4;
                    color_ids[3] = (obp1 & (0b11 << 6)) >> 6;
                }
                for (int i = 7; i >= 0; --i)
                {
                    std::uint8_t color_value = ((tile_high & (1 << 0)) << 1) | ((tile_low & (1 << 0)) >> 0);
                    if (i != 0)
                    {
                        color_value = ((tile_high & (1 << i)) >> (i - 1)) | ((tile_low & (1 << i)) >> i);
                    }
                    if (lcdc & (1 << 1) && color_value != 0)
                    {
                        if (x_flip && x_pos - (8 - i) >= 0 && x_pos - (8 - i) <= 159)
                        {
                            if (bg_window_over_obj && scanline_color_ids[x_pos - (8 - i)] != 0)
                            {
                                continue;
                            }
                            scanline[x_pos - (8 - i)] = color_ids[color_value];
                        }
                        else if (!x_flip && x_pos - (i + 1) >= 0 && x_pos - (i + 1) <= 159)
                        {
                            if (bg_window_over_obj && scanline_color_ids[x_pos - (i + 1)] != 0)
                            {
                                continue;
                            }
                            scanline[x_pos - (i + 1)] = color_ids[color_value];
                        }
                    }
                }
            }
            std::copy(scanline.begin(), scanline.end(), frame.begin() + ly * 160);
            sprite_array.clear();
            scanline.fill(0);
            cycle += 172;
            mode = 0;
        }
    }
    else if (mode == 1)
    {
        std::uint8_t ly_register = read(0xff44);
        if (ly_register == 153)
        {
            mode = 2;
            write(0xff44, 0);
        }
        else if (cycle % 456 == 0)
        {
            write(0xff44, ++ly_register);
        }
    }
    --cycle;
}