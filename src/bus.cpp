#include <array>
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iomanip>
#include <bitset>

#include "bus.h"

bus::bus()
{
    vram.fill(0);
    wram.fill(0);
    oam.fill(0);
    hram.fill(0);
    for (size_t i = 0; i < 0x2000; ++i)
    {
        ext_ram.push_back(0);
    }
    io_registers.fill(0);
    io_registers[0x00] = 0xcf;
    io_registers[0x01] = 0x00;
    io_registers[0x02] = 0x7e;
    io_registers[0x04] = 0xAB;
    io_registers[0x05] = 0x00;
    io_registers[0x06] = 0x00;
    io_registers[0x07] = 0xf8;
    io_registers[0x0f] = 0xe1;
    io_registers[0x10] = 0x80;
    io_registers[0x11] = 0xbf;
    io_registers[0x12] = 0xf3;
    io_registers[0x13] = 0xff;
    io_registers[0x14] = 0xbf;
    io_registers[0x16] = 0x3f;
    io_registers[0x17] = 0x00;
    io_registers[0x18] = 0xff;
    io_registers[0x19] = 0xbf;
    io_registers[0x1a] = 0x7f;
    io_registers[0x1b] = 0xff;
    io_registers[0x1c] = 0x9f;
    io_registers[0x1d] = 0xff;
    io_registers[0x1e] = 0xbf;
    io_registers[0x20] = 0xff;
    io_registers[0x21] = 0x00;
    io_registers[0x22] = 0x00;
    io_registers[0x23] = 0xbf;
    io_registers[0x24] = 0x77;
    io_registers[0x25] = 0xf3;
    io_registers[0x26] = 0xf1;
    io_registers[0x40] = 0x91;
    io_registers[0x41] = 0x85;
    io_registers[0x42] = 0x00;
    io_registers[0x43] = 0x00;
    io_registers[0x44] = 0x00;
    io_registers[0x45] = 0x00;
    io_registers[0x46] = 0xff;
    io_registers[0x47] = 0xfc;
    io_registers[0x48] = 0xff;
    io_registers[0x49] = 0xff;
    io_registers[0x4a] = 0x00;
    io_registers[0x4b] = 0x00;
    io_registers[0x4d] = 0xff;
    io_registers[0x4f] = 0xff;
    io_registers[0x51] = 0xff;
    io_registers[0x52] = 0xff;
    io_registers[0x53] = 0xff;
    io_registers[0x54] = 0xff;
    io_registers[0x55] = 0xff;
    io_registers[0x56] = 0xff;
    io_registers[0x68] = 0xff;
    io_registers[0x69] = 0xff;
    io_registers[0x6a] = 0xff;
    io_registers[0x6b] = 0xff;
    io_registers[0x70] = 0xff;
    ie_register = 0x00;

    rom_bank_index = 1;
    rom_bank_0_index = 0;
    ext_ram_bank_index = 0;
    n_rom_banks = 2;
    n_ram_banks = 0;
    ram_enabled = false;
    banking_mode = 0;

    cartridge_type = 0x00;

    dma_cycle = 0;
}

void bus::dma_clock()
{
    if (dma_cycle > 0)
    {
        --dma_cycle;
    }
}

std::uint8_t bus::read(std::uint16_t address)
{
    if (0x0000 <= address && address <= 0x3fff)
    {
        if (banking_mode == 0)
        {
            return rom[address];
        }
        else
        {
            return rom[rom_bank_0_index * 0x4000 + address];
        }
    }

    else if (0x4000 <= address && address <= 0x7fff)
    {
        return rom[rom_bank_index * 0x4000 + (address - 0x4000)];
    }

    else if (0x8000 <= address && address <= 0x9fff)
    {
        return vram[address - 0x8000];
    }

    else if (0xa000 <= address && address <= 0xbfff && ram_enabled)
    {
        if (banking_mode == 0)
        {
            return ext_ram[address - 0xa000];
        }
        else
        {
            return ext_ram[ext_ram_bank_index * 0x2000 + (address - 0xa000)];
        }
    }

    else if (0xc000 <= address && address <= 0xdfff)
    {
        return wram[address - 0xc000];
    }

    else if (0xe000 <= address && address <= 0xfdff)
    {
        return wram[address - 0xe000];
    }

    else if (0xfe00 <= address && address <= 0xfe9f)
    {
        return oam[address - 0xfe00];
    }

    else if (0xfea0 <= address && address <= 0xfeff)
    {
        return 0x00;
    }

    else if (0xff00 <= address && address <= 0xff7f)
    {
        return io_registers[address - 0xff00];
    }

    else if (0xff80 <= address && address <= 0xfffe)
    {
        return hram[address - 0xff80];
    }

    else if (address == 0xffff)
    {
        return ie_register;
    }
}

void bus::write(std::uint16_t address, std::uint8_t data)
{
    if (0x0000 <= address && address <= 0x1fff)
    {
        if ((data & 0xf) == 0x0a)
        {
            ram_enabled = true;
        }
        else
        {
            ram_enabled = false;
        }
    }

    else if (0x2000 <= address && address <= 0x3fff)
    {
        if (n_rom_banks == 2)
        {
            // no rom banking
        }
        else if (4 <= n_rom_banks && n_rom_banks <= 32)
        {
            std::uint8_t n_relevant_bits = std::log2(n_rom_banks);
            data = data & (0b11111 >> (5 - n_relevant_bits));
            if (data == 0)
            {
                rom_bank_index = 1;
            }
            else
            {
                rom_bank_index = data;
            }
        }
        else
        {
            data = data & 0b11111;
            if (data == 0)
            {
                rom_bank_index = (rom_bank_index & (11 << 5)) | 1;
            }
            else
            {
                rom_bank_index = (rom_bank_index & (11 << 5)) | data;
            }
        }
    }

    else if (0x4000 <= address && address <= 0x5fff)
    {
        data = data & 0b11;
        if (n_rom_banks >= 64)
        {
            rom_bank_index = (rom_bank_index & 0b11111) | (data << 5);
            rom_bank_0_index = (data << 5);
        }
        if (n_ram_banks >= 4)
        {
            ext_ram_bank_index = data;
        }
    }

    else if (0x6000 <= address && address <= 0x7fff)
    {
        data = data & 0b1;
        banking_mode = data;
    }

    else if (0x8000 <= address && address <= 0x9fff)
    {
        vram[address - 0x8000] = data;
    }

    else if (0xa000 <= address && address <= 0xbfff && ram_enabled)
    {
        if (banking_mode == 0)
        {
            ext_ram[address - 0xa000] = data;
        }
        else
        {
            ext_ram[ext_ram_bank_index * 0x2000 + (address - 0xa000)] = data;
        }
    }

    else if (0xc000 <= address && address <= 0xdfff)
    {
        wram[address - 0xc000] = data;
    }

    else if (0xe000 <= address && address <= 0xfdff)
    {
        wram[address - 0xe000] = data;
    }

    else if (0xfe00 <= address && address <= 0xfe9f)
    {
        oam[address - 0xfe00] = data;
    }

    else if (0xfea0 <= address && address <= 0xfeff)
    {
        // address space not usable
    }

    else if (0xff00 <= address && address <= 0xff7f)
    {
        if (address == 0xff00)
        {
            bool action = data & (1 << 5);
            bool direction = data & (1 << 4);
            if (!action)
            {
                io_registers[0] = io_registers[0] & ~(1 << 5);
                for (int i = 0; i < 4; ++i)
                {
                    if (action_buttons[i])
                    {
                        io_registers[0] = io_registers[0] & ~(1 << i);
                    }
                    else
                    {
                        io_registers[0] = io_registers[0] | (1 << i);
                    }
                }
            }
            else
            {
                io_registers[0] = io_registers[0] | (1 << 5);
            }
            if (!direction)
            {
                io_registers[0] = io_registers[0] & ~(1 << 4);
                for (int i = 0; i < 4; ++i)
                {
                    if (direction_buttons[i])
                    {
                        io_registers[0] = io_registers[0] & ~(1 << i);
                    }
                    else
                    {
                        io_registers[0] = io_registers[0] | (1 << i);
                    }
                }
            }
            else
            {
                io_registers[0] = io_registers[0] | (1 << 4);
            }
        }
        else if (address == 0xff04)
        {
            io_registers[address - 0xff00] = 0x00;
        }
        else if (address == 0xff46 && dma_cycle == 0)
        {
            dma_cycle += 160;
            io_registers[address - 0xff00] = data;
            std::uint16_t source = (data << 8);
            if (0x0000 <= source && source <= 0x3fff)
            {
                if (banking_mode == 0)
                {
                    std::copy(rom.begin() + source, rom.begin() + source + 0x9f, oam.begin());
                }
                else
                {
                    std::copy(rom.begin() + (rom_bank_0_index * 0x4000 + source), rom.begin() + (rom_bank_0_index * 0x4000 + source) + 0x9f, oam.begin());
                }
            }
            else if (0x4000 <= source && source <= 0x7fff)
            {
                std::copy(rom.begin() + (rom_bank_index * 0x4000 + (source - 0x4000)), rom.begin() + (rom_bank_index * 0x4000 + (source - 0x4000)) + 0x9f, oam.begin());
            }
            else if (0x8000 <= source && source <= 0x9fff)
            {
                std::copy(vram.begin() + (source - 0x8000), vram.begin() + (source - 0x8000) + 0x9f, oam.begin());
            }
            else if (0xa000 <= source && source <= 0xbfff)
            {
                if (banking_mode == 0)
                {
                    std::copy(ext_ram.begin() + (source - 0xa000), ext_ram.begin() + (source - 0xa000) + 0x9f, oam.begin());
                }
                else
                {
                    std::copy(ext_ram.begin() + (ext_ram_bank_index * 0x2000 + (source - 0xa000)), ext_ram.begin() + (ext_ram_bank_index * 0x2000 + (source - 0xa000)) + 0x9f, oam.begin());
                }
            }
            else if (0xc000 <= source && source <= 0xdfff)
            {
                std::copy(wram.begin() + (source - 0xc000), wram.begin() + (source - 0xc000) + 0x9f, oam.begin());
            }
        }
        else
        {
            io_registers[address - 0xff00] = data;
        }
    }

    else if (0xff80 <= address && address <= 0xfffe)
    {
        hram[address - 0xff80] = data;
    }

    else if (address == 0xffff)
    {
        ie_register = data;
    }
}

void bus::increment_div()
{
    std::uint8_t div = io_registers[0x04];
    io_registers[0x04] = ++div;
}

void bus::load_rom(std::string path)
{
    std::ifstream input(path, std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    rom = buffer;
    std::uint8_t cartridge_type = rom[0x0147];
    std::uint8_t rom_size_code = rom[0x0148];
    std::uint8_t ram_size_code = rom[0x0149];

    switch (rom_size_code)
    {
    case 0x00:
        n_rom_banks = 2;
        break;
    case 0x01:
        n_rom_banks = 4;
        break;
    case 0x02:
        n_rom_banks = 8;
        break;
    case 0x03:
        n_rom_banks = 16;
        break;
    case 0x04:
        n_rom_banks = 32;
        break;
    case 0x05:
        n_rom_banks = 64;
        break;
    case 0x06:
        n_rom_banks = 128;
        break;
    case 0x07:
        n_rom_banks = 256;
        break;
    case 0x08:
        n_rom_banks = 512;
        break;
    }

    switch (ram_size_code)
    {
    case 0x00:
        n_ram_banks = 0;
        break;
    case 0x01:
        n_ram_banks = 0;
        break;
    case 0x02:
        n_ram_banks = 1;
        break;
    case 0x03:
        n_ram_banks = 4;
        break;
    case 0x04:
        n_ram_banks = 16;
        break;
    case 0x05:
        n_ram_banks = 8;
        break;
    }
}

void bus::load_ext_ram(std::string path)
{
    std::ifstream input(path, std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    ext_ram = buffer;
}

void bus::set_action_button(std::uint8_t index, bool value)
{
    action_buttons[index] = value;
}

void bus::set_direction_button(std::uint8_t index, bool value)
{
    direction_buttons[index] = value;
}