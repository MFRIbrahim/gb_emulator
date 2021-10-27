#pragma once
#include <array>
#include <cstdint>
#include <vector>

class bus
{
private:
    std::vector<uint8_t> rom;
    std::array<uint8_t, 0x2000> vram;
    std::vector<uint8_t> ext_ram;
    std::array<uint8_t, 0x2000> wram;
    std::array<uint8_t, 0xa0> oam;
    std::array<uint8_t, 0x80> io_registers;
    std::array<uint8_t, 0x7f> hram;
    std::uint8_t ie_register;

    std::uint16_t rom_bank_index;
    std::uint8_t rom_bank_0_index;
    std::uint8_t ext_ram_bank_index;
    std::uint16_t n_rom_banks;
    std::uint8_t n_ram_banks;

    bool ram_enabled;
    bool banking_mode;

    std::uint8_t cartridge_type;

    std::uint8_t dma_cycle;

    std::array<bool, 4> action_buttons;    // A, B, Select, Start
    std::array<bool, 4> direction_buttons; // Right, Left, Up, Down

public:
    bus();
    std::uint8_t read(std::uint16_t address);
    void write(std::uint16_t address, std::uint8_t data);
    void increment_div();
    void load_rom(std::string path);
    void load_ext_ram(std::string path);

    void set_action_button(std::uint8_t index, bool value);
    void set_direction_button(std::uint8_t index, bool value);

    void dma_clock();
};