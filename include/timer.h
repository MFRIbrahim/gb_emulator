#pragma once
#include <cstdint>

class bus;

class timer
{
private:
    std::uint16_t div_counter;
    std::uint16_t tima_counter;
    std::uint8_t tima_delay;

    bus *gb_bus;

public:
    timer();

    void connect_bus(bus *b);

    void clock();

    std::uint8_t read(std::uint16_t address);
    void write(std::uint16_t address, std::uint8_t data);
};