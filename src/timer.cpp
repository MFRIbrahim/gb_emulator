#include <cstdint>

#include "timer.h"
#include "bus.h"

timer::timer()
{
    div_counter = 0;
    tima_counter = 0;
    tima_delay = 0;
}

void timer::connect_bus(bus *b)
{
    gb_bus = b;
}

std::uint8_t timer::read(std::uint16_t address)
{
    return gb_bus->read(address);
}

void timer::write(std::uint16_t address, std::uint8_t data)
{
    return gb_bus->write(address, data);
}

void timer::clock()
{
    ++div_counter;
    if (div_counter == 256)
    {
        div_counter -= 256;
        gb_bus->increment_div();
    }
    std::uint8_t tac = read(0xff07);
    if (tac & (1 << 2))
    {
        ++tima_counter;
        std::uint32_t freq = 4096;
        if (tac & 0x01 == 0x01)
        {
            freq = 262144;
        }
        else if (tac & 0x02 == 0x02)
        {
            freq = 65536;
        }
        else if (tac & 0x03 == 0x03)
        {
            freq = 16384;
        }

        std::uint8_t tima = read(0xff05);
        if (tima_delay == 0)
        {
            while (tima_counter >= (4194304 / freq))
            {
                tima_counter -= (4194304 / freq);
                ++tima;
                if (tima == 0x00)
                {
                    write(0xff05, 0x00);
                    ++tima_delay;
                }
                else
                {
                    write(0xff05, tima);
                }
            }
        }
        else
        {
            ++tima_delay;
            if (tima_delay == 4)
            {
                write(0xff0f, read(0xff0f) | (1 << 2));
                write(0xff05, read(0xff06));
                tima_delay = 0;
            }
        }
    }
}
