#include <cstdint>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "cpu.h"
#include "bus.h"

cpu::cpu()
{
    cycle = 0;
    ime_flag = false;
    halted = false;

    a = 0x01;
    f = 0xb0;
    b = 0x00;
    c = 0x13;
    d = 0x00;
    e = 0xd8;
    h = 0x01;
    l = 0x4d;

    sp = 0xfffe;
    pc = 0x0100;
}

void cpu::connect_bus(bus *b)
{
    gb_bus = b;
    std::uint8_t header_cheksum = read(0x014d);
    if (header_cheksum == 0x00)
    {
        f = f & ~(1 << 4);
        f = f & ~(1 << 5);
    }
    else
    {
        f = f | (1 << 4);
        f = f | (1 << 5);
    }
}

std::uint8_t cpu::read(std::uint16_t address)
{
    return gb_bus->read(address);
}

void cpu::write(std::uint16_t address, std::uint8_t data)
{
    return gb_bus->write(address, data);
}

void cpu::handle_interrupt()
{
    std::uint8_t ie_register = read(0xffff);
    std::uint8_t if_register = read(0xff0f);
    if (ie_register & if_register)
    {
        if (ime_flag)
        {
            bool vblank = ((ie_register & (1 << 0)) & (if_register & (1 << 0)));
            bool lcd_stat = ((ie_register & (1 << 1)) & (if_register & (1 << 1)));
            bool timer = ((ie_register & (1 << 2)) & (if_register & (1 << 2)));
            bool serial = ((ie_register & (1 << 3)) & (if_register & (1 << 3)));
            bool joypad = ((ie_register & (1 << 4)) & (if_register & (1 << 4)));

            if (vblank)
            {
                std::uint8_t high = pc >> 8;
                std::uint8_t low = pc;
                --sp;
                write(sp, high);
                --sp;
                write(sp, low);
                pc = 0x0000 + 0x40;
                ime_flag = false;
                write(0xff0f, if_register & ~(1 << 0));
                cycle += 20;
            }

            else if (lcd_stat)
            {
                std::uint8_t high = pc >> 8;
                std::uint8_t low = pc;
                --sp;
                write(sp, high);
                --sp;
                write(sp, low);
                pc = 0x0000 + 0x48;
                ime_flag = false;
                write(0xff0f, if_register & ~(1 << 1));
                cycle += 20;
            }

            else if (timer)
            {
                std::uint8_t high = pc >> 8;
                std::uint8_t low = pc;
                --sp;
                write(sp, high);
                --sp;
                write(sp, low);
                pc = 0x0000 + 0x50;
                ime_flag = false;
                write(0xff0f, if_register & ~(1 << 2));
                cycle += 20;
            }

            else if (serial)
            {
                std::uint8_t high = pc >> 8;
                std::uint8_t low = pc;
                --sp;
                write(sp, high);
                --sp;
                write(sp, low);
                pc = 0x0000 + 0x58;
                ime_flag = false;
                write(0xff0f, if_register & ~(1 << 3));
                cycle += 20;
            }

            else if (joypad)
            {
                std::uint8_t high = pc >> 8;
                std::uint8_t low = pc;
                --sp;
                write(sp, high);
                --sp;
                write(sp, low);
                pc = 0x0000 + 0x60;
                ime_flag = false;
                write(0xff0f, if_register & ~(1 << 4));
                cycle += 20;
            }
        }
        else
        {
            cycle += 8;
        }
        halted = false;
    }
}

void cpu::clock()
{
    if (cycle == 0 && !halted)
    {
        std::uint8_t byte_1 = read(pc);
        ++pc;
        switch (byte_1)
        {
        case 0x00:
            cycle += nop();
            break;
        case 0x01:
            cycle += ld_bc_d16();
            break;
        case 0x02:
            cycle += ld_addr_bc_a();
            break;
        case 0x03:
            cycle += inc_bc();
            break;
        case 0x04:
            cycle += inc_b();
            break;
        case 0x05:
            cycle += dec_b();
            break;
        case 0x06:
            cycle += ld_b_d8();
            break;
        case 0x07:
            cycle += rlca();
            break;
        case 0x08:
            cycle += ld_a16_sp();
            break;
        case 0x09:
            cycle += add_hl_bc();
            break;
        case 0x0a:
            cycle += ld_a_addr_bc();
            break;
        case 0x0b:
            cycle += dec_bc();
            break;
        case 0x0c:
            cycle += inc_c();
            break;
        case 0x0d:
            cycle += dec_c();
            break;
        case 0x0e:
            cycle += ld_c_d8();
            break;
        case 0x0f:
            cycle += rrca();
            break;
        case 0x10:
            cycle += stop();
            break;
        case 0x11:
            cycle += ld_de_d16();
            break;
        case 0x12:
            cycle += ld_addr_de_a();
            break;
        case 0x13:
            cycle += inc_de();
            break;
        case 0x14:
            cycle += inc_d();
            break;
        case 0x15:
            cycle += dec_d();
            break;
        case 0x16:
            cycle += ld_d_d8();
            break;
        case 0x17:
            cycle += rla();
            break;
        case 0x18:
            cycle += jr_r8();
            break;
        case 0x19:
            cycle += add_hl_de();
            break;
        case 0x1a:
            cycle += ld_a_addr_de();
            break;
        case 0x1b:
            cycle += dec_de();
            break;
        case 0x1c:
            cycle += inc_e();
            break;
        case 0x1d:
            cycle += dec_e();
            break;
        case 0x1e:
            cycle += ld_e_d8();
            break;
        case 0x1f:
            cycle += rra();
            break;
        case 0x20:
            cycle += jr_nz_r8();
            break;
        case 0x21:
            cycle += ld_hl_d16();
            break;
        case 0x22:
            cycle += ldi_addr_hl_a();
            break;
        case 0x23:
            cycle += inc_hl();
            break;
        case 0x24:
            cycle += inc_h();
            break;
        case 0x25:
            cycle += dec_h();
            break;
        case 0x26:
            cycle += ld_h_d8();
            break;
        case 0x27:
            cycle += daa();
            break;
        case 0x28:
            cycle += jr_z_r8();
            break;
        case 0x29:
            cycle += add_hl_hl();
            break;
        case 0x2a:
            cycle += ldi_a_addr_hl();
            break;
        case 0x2b:
            cycle += dec_hl();
            break;
        case 0x2c:
            cycle += inc_l();
            break;
        case 0x2d:
            cycle += dec_l();
            break;
        case 0x2e:
            cycle += ld_l_d8();
            break;
        case 0x2f:
            cycle += cpl();
            break;
        case 0x30:
            cycle += jr_nc_r8();
            break;
        case 0x31:
            cycle += ld_sp_d16();
            break;
        case 0x32:
            cycle += ldd_addr_hl_a();
            break;
        case 0x33:
            cycle += inc_sp();
            break;
        case 0x34:
            cycle += inc_addr_hl();
            break;
        case 0x35:
            cycle += dec_addr_hl();
            break;
        case 0x36:
            cycle += ld_addr_hl_d8();
            break;
        case 0x37:
            cycle += scf();
            break;
        case 0x38:
            cycle += jr_c_r8();
            break;
        case 0x39:
            cycle += add_hl_sp();
            break;
        case 0x3a:
            cycle += ldd_a_addr_hl();
            break;
        case 0x3b:
            cycle += dec_sp();
            break;
        case 0x3c:
            cycle += inc_a();
            break;
        case 0x3d:
            cycle += dec_a();
            break;
        case 0x3e:
            cycle += ld_a_d8();
            break;
        case 0x3f:
            cycle += ccf();
            break;
        case 0x40:
            cycle += ld_b_b();
            break;
        case 0x41:
            cycle += ld_b_c();
            break;
        case 0x42:
            cycle += ld_b_d();
            break;
        case 0x43:
            cycle += ld_b_e();
            break;
        case 0x44:
            cycle += ld_b_h();
            break;
        case 0x45:
            cycle += ld_b_l();
            break;
        case 0x46:
            cycle += ld_b_addr_hl();
            break;
        case 0x47:
            cycle += ld_b_a();
            break;
        case 0x48:
            cycle += ld_c_b();
            break;
        case 0x49:
            cycle += ld_c_c();
            break;
        case 0x4a:
            cycle += ld_c_d();
            break;
        case 0x4b:
            cycle += ld_c_e();
            break;
        case 0x4c:
            cycle += ld_c_h();
            break;
        case 0x4d:
            cycle += ld_c_l();
            break;
        case 0x4e:
            cycle += ld_c_addr_hl();
            break;
        case 0x4f:
            cycle += ld_c_a();
            break;
        case 0x50:
            cycle += ld_d_b();
            break;
        case 0x51:
            cycle += ld_d_c();
            break;
        case 0x52:
            cycle += ld_d_d();
            break;
        case 0x53:
            cycle += ld_d_e();
            break;
        case 0x54:
            cycle += ld_d_h();
            break;
        case 0x55:
            cycle += ld_d_l();
            break;
        case 0x56:
            cycle += ld_d_addr_hl();
            break;
        case 0x57:
            cycle += ld_d_a();
            break;
        case 0x58:
            cycle += ld_e_b();
            break;
        case 0x59:
            cycle += ld_e_c();
            break;
        case 0x5a:
            cycle += ld_e_d();
            break;
        case 0x5b:
            cycle += ld_e_e();
            break;
        case 0x5c:
            cycle += ld_e_h();
            break;
        case 0x5d:
            cycle += ld_e_l();
            break;
        case 0x5e:
            cycle += ld_e_addr_hl();
            break;
        case 0x5f:
            cycle += ld_e_a();
            break;
        case 0x60:
            cycle += ld_h_b();
            break;
        case 0x61:
            cycle += ld_h_c();
            break;
        case 0x62:
            cycle += ld_h_d();
            break;
        case 0x63:
            cycle += ld_h_e();
            break;
        case 0x64:
            cycle += ld_h_h();
            break;
        case 0x65:
            cycle += ld_h_l();
            break;
        case 0x66:
            cycle += ld_h_addr_hl();
            break;
        case 0x67:
            cycle += ld_h_a();
            break;
        case 0x68:
            cycle += ld_l_b();
            break;
        case 0x69:
            cycle += ld_l_c();
            break;
        case 0x6a:
            cycle += ld_l_d();
            break;
        case 0x6b:
            cycle += ld_l_e();
            break;
        case 0x6c:
            cycle += ld_l_h();
            break;
        case 0x6d:
            cycle += ld_l_l();
            break;
        case 0x6e:
            cycle += ld_l_addr_hl();
            break;
        case 0x6f:
            cycle += ld_l_a();
            break;
        case 0x70:
            cycle += ld_addr_hl_b();
            break;
        case 0x71:
            cycle += ld_addr_hl_c();
            break;
        case 0x72:
            cycle += ld_addr_hl_d();
            break;
        case 0x73:
            cycle += ld_addr_hl_e();
            break;
        case 0x74:
            cycle += ld_addr_hl_h();
            break;
        case 0x75:
            cycle += ld_addr_hl_l();
            break;
        case 0x76:
            cycle += halt();
            break;
        case 0x77:
            cycle += ld_addr_hl_a();
            break;
        case 0x78:
            cycle += ld_a_b();
            break;
        case 0x79:
            cycle += ld_a_c();
            break;
        case 0x7a:
            cycle += ld_a_d();
            break;
        case 0x7b:
            cycle += ld_a_e();
            break;
        case 0x7c:
            cycle += ld_a_h();
            break;
        case 0x7d:
            cycle += ld_a_l();
            break;
        case 0x7e:
            cycle += ld_a_addr_hl();
            break;
        case 0x7f:
            cycle += ld_a_a();
            break;
        case 0x80:
            cycle += add_a_b();
            break;
        case 0x81:
            cycle += add_a_c();
            break;
        case 0x82:
            cycle += add_a_d();
            break;
        case 0x83:
            cycle += add_a_e();
            break;
        case 0x84:
            cycle += add_a_h();
            break;
        case 0x85:
            cycle += add_a_l();
            break;
        case 0x86:
            cycle += add_a_addr_hl();
            break;
        case 0x87:
            cycle += add_a_a();
            break;
        case 0x88:
            cycle += adc_a_b();
            break;
        case 0x89:
            cycle += adc_a_c();
            break;
        case 0x8a:
            cycle += adc_a_d();
            break;
        case 0x8b:
            cycle += adc_a_e();
            break;
        case 0x8c:
            cycle += adc_a_h();
            break;
        case 0x8d:
            cycle += adc_a_l();
            break;
        case 0x8e:
            cycle += adc_a_addr_hl();
            break;
        case 0x8f:
            cycle += adc_a_a();
            break;
        case 0x90:
            cycle += sub_b();
            break;
        case 0x91:
            cycle += sub_c();
            break;
        case 0x92:
            cycle += sub_d();
            break;
        case 0x93:
            cycle += sub_e();
            break;
        case 0x94:
            cycle += sub_h();
            break;
        case 0x95:
            cycle += sub_l();
            break;
        case 0x96:
            cycle += sub_addr_hl();
            break;
        case 0x97:
            cycle += sub_a();
            break;
        case 0x98:
            cycle += sbc_a_b();
            break;
        case 0x99:
            cycle += sbc_a_c();
            break;
        case 0x9a:
            cycle += sbc_a_d();
            break;
        case 0x9b:
            cycle += sbc_a_e();
            break;
        case 0x9c:
            cycle += sbc_a_h();
            break;
        case 0x9d:
            cycle += sbc_a_l();
            break;
        case 0x9e:
            cycle += sbc_a_addr_hl();
            break;
        case 0x9f:
            cycle += sbc_a_a();
            break;
        case 0xa0:
            cycle += and_b();
            break;
        case 0xa1:
            cycle += and_c();
            break;
        case 0xa2:
            cycle += and_d();
            break;
        case 0xa3:
            cycle += and_e();
            break;
        case 0xa4:
            cycle += and_h();
            break;
        case 0xa5:
            cycle += and_l();
            break;
        case 0xa6:
            cycle += and_addr_hl();
            break;
        case 0xa7:
            cycle += and_a();
            break;
        case 0xa8:
            cycle += xor_b();
            break;
        case 0xa9:
            cycle += xor_c();
            break;
        case 0xaa:
            cycle += xor_d();
            break;
        case 0xab:
            cycle += xor_e();
            break;
        case 0xac:
            cycle += xor_h();
            break;
        case 0xad:
            cycle += xor_l();
            break;
        case 0xae:
            cycle += xor_addr_hl();
            break;
        case 0xaf:
            cycle += xor_a();
            break;
        case 0xb0:
            cycle += or_b();
            break;
        case 0xb1:
            cycle += or_c();
            break;
        case 0xb2:
            cycle += or_d();
            break;
        case 0xb3:
            cycle += or_e();
            break;
        case 0xb4:
            cycle += or_h();
            break;
        case 0xb5:
            cycle += or_l();
            break;
        case 0xb6:
            cycle += or_addr_hl();
            break;
        case 0xb7:
            cycle += or_a();
            break;
        case 0xb8:
            cycle += cp_b();
            break;
        case 0xb9:
            cycle += cp_c();
            break;
        case 0xba:
            cycle += cp_d();
            break;
        case 0xbb:
            cycle += cp_e();
            break;
        case 0xbc:
            cycle += cp_h();
            break;
        case 0xbd:
            cycle += cp_l();
            break;
        case 0xbe:
            cycle += cp_addr_hl();
            break;
        case 0xbf:
            cycle += cp_a();
            break;
        case 0xc0:
            cycle += ret_nz();
            break;
        case 0xc1:
            cycle += pop_bc();
            break;
        case 0xc2:
            cycle += jp_nz_a16();
            break;
        case 0xc3:
            cycle += jp_a16();
            break;
        case 0xc4:
            cycle += call_nz_a16();
            break;
        case 0xc5:
            cycle += push_bc();
            break;
        case 0xc6:
            cycle += add_a_d8();
            break;
        case 0xc7:
            cycle += rst_00h();
            break;
        case 0xc8:
            cycle += ret_z();
            break;
        case 0xc9:
            cycle += ret();
            break;
        case 0xca:
            cycle += jp_z_a16();
            break;
        case 0xcc:
            cycle += call_z_a16();
            break;
        case 0xcd:
            cycle += call_a16();
            break;
        case 0xce:
            cycle += adc_a_d8();
            break;
        case 0xcf:
            cycle += rst_08h();
            break;
        case 0xd0:
            cycle += ret_nc();
            break;
        case 0xd1:
            cycle += pop_de();
            break;
        case 0xd2:
            cycle += jp_nc_a16();
            break;
        case 0xd3:
            cycle += invalid();
            break;
        case 0xd4:
            cycle += call_nc_a16();
            break;
        case 0xd5:
            cycle += push_de();
            break;
        case 0xd6:
            cycle += sub_d8();
            break;
        case 0xd7:
            cycle += rst_10h();
            break;
        case 0xd8:
            cycle += ret_c();
            break;
        case 0xd9:
            cycle += reti();
            break;
        case 0xda:
            cycle += jp_c_a16();
            break;
        case 0xdb:
            cycle += invalid();
            break;
        case 0xdc:
            cycle += call_c_a16();
            break;
        case 0xdd:
            cycle += invalid();
            break;
        case 0xde:
            cycle += sbc_a_d8();
            break;
        case 0xdf:
            cycle += rst_18h();
            break;
        case 0xe0:
            cycle += ldh_a8_a();
            break;
        case 0xe1:
            cycle += pop_hl();
            break;
        case 0xe2:
            cycle += ld_addr_c_a();
            break;
        case 0xe3:
            cycle += invalid();
            break;
        case 0xe4:
            cycle += invalid();
            break;
        case 0xe5:
            cycle += push_hl();
            break;
        case 0xe6:
            cycle += and_d8();
            break;
        case 0xe7:
            cycle += rst_20h();
            break;
        case 0xe8:
            cycle += add_sp_r8();
            break;
        case 0xe9:
            cycle += jp_hl();
            break;
        case 0xea:
            cycle += ld_a16_a();
            break;
        case 0xeb:
            cycle += invalid();
            break;
        case 0xec:
            cycle += invalid();
            break;
        case 0xed:
            cycle += invalid();
            break;
        case 0xee:
            cycle += xor_d8();
            break;
        case 0xef:
            cycle += rst_28h();
            break;
        case 0xf0:
            cycle += ldh_a_a8();
            break;
        case 0xf1:
            cycle += pop_af();
            break;
        case 0xf2:
            cycle += ld_a_addr_c();
            break;
        case 0xf3:
            cycle += di();
            break;
        case 0xf4:
            cycle += invalid();
            break;
        case 0xf5:
            cycle += push_af();
            break;
        case 0xf6:
            cycle += or_d8();
            break;
        case 0xf7:
            cycle += rst_30h();
            break;
        case 0xf8:
            cycle += ld_hl_sp_plus_r8();
            break;
        case 0xf9:
            cycle += ld_sp_hl();
            break;
        case 0xfa:
            cycle += ld_a_a16();
            break;
        case 0xfb:
            cycle += ei();
            break;
        case 0xfc:
            cycle += invalid();
            break;
        case 0xfd:
            cycle = invalid();
            break;
        case 0xfe:
            cycle += cp_d8();
            break;
        case 0xff:
            cycle += rst_38h();
            break;

        case 0xcb:
            std::uint8_t byte_2 = read(pc);
            ++pc;

            switch (byte_2)
            {
            case 0x00:
                cycle += rlc_b();
                break;
            case 0x01:
                cycle += rlc_c();
                break;
            case 0x02:
                cycle += rlc_d();
                break;
            case 0x03:
                cycle += rlc_e();
                break;
            case 0x04:
                cycle += rlc_h();
                break;
            case 0x05:
                cycle += rlc_l();
                break;
            case 0x06:
                cycle += rlc_addr_hl();
                break;
            case 0x07:
                cycle += rlc_a();
                break;
            case 0x08:
                cycle += rrc_b();
                break;
            case 0x09:
                cycle += rrc_c();
                break;
            case 0x0a:
                cycle += rrc_d();
                break;
            case 0x0b:
                cycle += rrc_e();
                break;
            case 0x0c:
                cycle += rrc_h();
                break;
            case 0x0d:
                cycle += rrc_l();
                break;
            case 0x0e:
                cycle += rrc_addr_hl();
                break;
            case 0x0f:
                cycle += rrc_a();
                break;
            case 0x10:
                cycle += rl_b();
                break;
            case 0x11:
                cycle += rl_c();
                break;
            case 0x12:
                cycle += rl_d();
                break;
            case 0x13:
                cycle += rl_e();
                break;
            case 0x14:
                cycle += rl_h();
                break;
            case 0x15:
                cycle += rl_l();
                break;
            case 0x16:
                cycle += rl_addr_hl();
                break;
            case 0x17:
                cycle += rl_a();
                break;
            case 0x18:
                cycle += rr_b();
                break;
            case 0x19:
                cycle += rr_c();
                break;
            case 0x1a:
                cycle += rr_d();
                break;
            case 0x1b:
                cycle += rr_e();
                break;
            case 0x1c:
                cycle += rr_h();
                break;
            case 0x1d:
                cycle += rr_l();
                break;
            case 0x1e:
                cycle += rr_addr_hl();
                break;
            case 0x1f:
                cycle += rr_a();
                break;
            case 0x20:
                cycle += sla_b();
                break;
            case 0x21:
                cycle += sla_c();
                break;
            case 0x22:
                cycle += sla_d();
                break;
            case 0x23:
                cycle += sla_e();
                break;
            case 0x24:
                cycle += sla_h();
                break;
            case 0x25:
                cycle += sla_l();
                break;
            case 0x26:
                cycle += sla_addr_hl();
                break;
            case 0x27:
                cycle += sla_a();
                break;
            case 0x28:
                cycle += sra_b();
                break;
            case 0x29:
                cycle += sra_c();
                break;
            case 0x2a:
                cycle += sra_d();
                break;
            case 0x2b:
                cycle += sra_e();
                break;
            case 0x2c:
                cycle += sra_h();
                break;
            case 0x2d:
                cycle += sra_l();
                break;
            case 0x2e:
                cycle += sra_addr_hl();
                break;
            case 0x2f:
                cycle += sra_a();
                break;
            case 0x30:
                cycle += swap_b();
                break;
            case 0x31:
                cycle += swap_c();
                break;
            case 0x32:
                cycle += swap_d();
                break;
            case 0x33:
                cycle += swap_e();
                break;
            case 0x34:
                cycle += swap_h();
                break;
            case 0x35:
                cycle += swap_l();
                break;
            case 0x36:
                cycle += swap_addr_hl();
                break;
            case 0x37:
                cycle += swap_a();
                break;
            case 0x38:
                cycle += srl_b();
                break;
            case 0x39:
                cycle += srl_c();
                break;
            case 0x3a:
                cycle += srl_d();
                break;
            case 0x3b:
                cycle += srl_e();
                break;
            case 0x3c:
                cycle += srl_h();
                break;
            case 0x3d:
                cycle += srl_l();
                break;
            case 0x3e:
                cycle += srl_addr_hl();
                break;
            case 0x3f:
                cycle += srl_a();
                break;
            case 0x40:
                cycle += bit_0_b();
                break;
            case 0x41:
                cycle += bit_0_c();
                break;
            case 0x42:
                cycle += bit_0_d();
                break;
            case 0x43:
                cycle += bit_0_e();
                break;
            case 0x44:
                cycle += bit_0_h();
                break;
            case 0x45:
                cycle += bit_0_l();
                break;
            case 0x46:
                cycle += bit_0_addr_hl();
                break;
            case 0x47:
                cycle += bit_0_a();
                break;
            case 0x48:
                cycle += bit_1_b();
                break;
            case 0x49:
                cycle += bit_1_c();
                break;
            case 0x4a:
                cycle += bit_1_d();
                break;
            case 0x4b:
                cycle += bit_1_e();
                break;
            case 0x4c:
                cycle += bit_1_h();
                break;
            case 0x4d:
                cycle += bit_1_l();
                break;
            case 0x4e:
                cycle += bit_1_addr_hl();
                break;
            case 0x4f:
                cycle += bit_1_a();
                break;
            case 0x50:
                cycle += bit_2_b();
                break;
            case 0x51:
                cycle += bit_2_c();
                break;
            case 0x52:
                cycle += bit_2_d();
                break;
            case 0x53:
                cycle += bit_2_e();
                break;
            case 0x54:
                cycle += bit_2_h();
                break;
            case 0x55:
                cycle += bit_2_l();
                break;
            case 0x56:
                cycle += bit_2_addr_hl();
                break;
            case 0x57:
                cycle += bit_2_a();
                break;
            case 0x58:
                cycle += bit_3_b();
                break;
            case 0x59:
                cycle += bit_3_c();
                break;
            case 0x5a:
                cycle += bit_3_d();
                break;
            case 0x5b:
                cycle += bit_3_e();
                break;
            case 0x5c:
                cycle += bit_3_h();
                break;
            case 0x5d:
                cycle += bit_3_l();
                break;
            case 0x5e:
                cycle += bit_3_addr_hl();
                break;
            case 0x5f:
                cycle += bit_3_a();
                break;
            case 0x60:
                cycle += bit_4_b();
                break;
            case 0x61:
                cycle += bit_4_c();
                break;
            case 0x62:
                cycle += bit_4_d();
                break;
            case 0x63:
                cycle += bit_4_e();
                break;
            case 0x64:
                cycle += bit_4_h();
                break;
            case 0x65:
                cycle += bit_4_l();
                break;
            case 0x66:
                cycle += bit_4_addr_hl();
                break;
            case 0x67:
                cycle += bit_4_a();
                break;
            case 0x68:
                cycle += bit_5_b();
                break;
            case 0x69:
                cycle += bit_5_c();
                break;
            case 0x6a:
                cycle += bit_5_d();
                break;
            case 0x6b:
                cycle += bit_5_e();
                break;
            case 0x6c:
                cycle += bit_5_h();
                break;
            case 0x6d:
                cycle += bit_5_l();
                break;
            case 0x6e:
                cycle += bit_5_addr_hl();
                break;
            case 0x6f:
                cycle += bit_5_a();
                break;
            case 0x70:
                cycle += bit_6_b();
                break;
            case 0x71:
                cycle += bit_6_c();
                break;
            case 0x72:
                cycle += bit_6_d();
                break;
            case 0x73:
                cycle += bit_6_e();
                break;
            case 0x74:
                cycle += bit_6_h();
                break;
            case 0x75:
                cycle += bit_6_l();
                break;
            case 0x76:
                cycle += bit_6_addr_hl();
                break;
            case 0x77:
                cycle += bit_6_a();
                break;
            case 0x78:
                cycle += bit_7_b();
                break;
            case 0x79:
                cycle += bit_7_c();
                break;
            case 0x7a:
                cycle += bit_7_d();
                break;
            case 0x7b:
                cycle += bit_7_e();
                break;
            case 0x7c:
                cycle += bit_7_h();
                break;
            case 0x7d:
                cycle += bit_7_l();
                break;
            case 0x7e:
                cycle += bit_7_addr_hl();
                break;
            case 0x7f:
                cycle += bit_7_a();
                break;
            case 0x80:
                cycle += res_0_b();
                break;
            case 0x81:
                cycle += res_0_c();
                break;
            case 0x82:
                cycle += res_0_d();
                break;
            case 0x83:
                cycle += res_0_e();
                break;
            case 0x84:
                cycle += res_0_h();
                break;
            case 0x85:
                cycle += res_0_l();
                break;
            case 0x86:
                cycle += res_0_addr_hl();
                break;
            case 0x87:
                cycle += res_0_a();
                break;
            case 0x88:
                cycle += res_1_b();
                break;
            case 0x89:
                cycle += res_1_c();
                break;
            case 0x8a:
                cycle += res_1_d();
                break;
            case 0x8b:
                cycle += res_1_e();
                break;
            case 0x8c:
                cycle += res_1_h();
                break;
            case 0x8d:
                cycle += res_1_l();
                break;
            case 0x8e:
                cycle += res_1_addr_hl();
                break;
            case 0x8f:
                cycle += res_1_a();
                break;
            case 0x90:
                cycle += res_2_b();
                break;
            case 0x91:
                cycle += res_2_c();
                break;
            case 0x92:
                cycle += res_2_d();
                break;
            case 0x93:
                cycle += res_2_e();
                break;
            case 0x94:
                cycle += res_2_h();
                break;
            case 0x95:
                cycle += res_2_l();
                break;
            case 0x96:
                cycle += res_2_addr_hl();
                break;
            case 0x97:
                cycle += res_2_a();
                break;
            case 0x98:
                cycle += res_3_b();
                break;
            case 0x99:
                cycle += res_3_c();
                break;
            case 0x9a:
                cycle += res_3_d();
                break;
            case 0x9b:
                cycle += res_3_e();
                break;
            case 0x9c:
                cycle += res_3_h();
                break;
            case 0x9d:
                cycle += res_3_l();
                break;
            case 0x9e:
                cycle += res_3_addr_hl();
                break;
            case 0x9f:
                cycle += res_3_a();
                break;
            case 0xa0:
                cycle += res_4_b();
                break;
            case 0xa1:
                cycle += res_4_c();
                break;
            case 0xa2:
                cycle += res_4_d();
                break;
            case 0xa3:
                cycle += res_4_e();
                break;
            case 0xa4:
                cycle += res_4_h();
                break;
            case 0xa5:
                cycle += res_4_l();
                break;
            case 0xa6:
                cycle += res_4_addr_hl();
                break;
            case 0xa7:
                cycle += res_4_a();
                break;
            case 0xa8:
                cycle += res_5_b();
                break;
            case 0xa9:
                cycle += res_5_c();
                break;
            case 0xaa:
                cycle += res_5_d();
                break;
            case 0xab:
                cycle += res_5_e();
                break;
            case 0xac:
                cycle += res_5_h();
                break;
            case 0xad:
                cycle += res_5_l();
                break;
            case 0xae:
                cycle += res_5_addr_hl();
                break;
            case 0xaf:
                cycle += res_5_a();
                break;
            case 0xb0:
                cycle += res_6_b();
                break;
            case 0xb1:
                cycle += res_6_c();
                break;
            case 0xb2:
                cycle += res_6_d();
                break;
            case 0xb3:
                cycle += res_6_e();
                break;
            case 0xb4:
                cycle += res_6_h();
                break;
            case 0xb5:
                cycle += res_6_l();
                break;
            case 0xb6:
                cycle += res_6_addr_hl();
                break;
            case 0xb7:
                cycle += res_6_a();
                break;
            case 0xb8:
                cycle += res_7_b();
                break;
            case 0xb9:
                cycle += res_7_c();
                break;
            case 0xba:
                cycle += res_7_d();
                break;
            case 0xbb:
                cycle += res_7_e();
                break;
            case 0xbc:
                cycle += res_7_h();
                break;
            case 0xbd:
                cycle += res_7_l();
                break;
            case 0xbe:
                cycle += res_7_addr_hl();
                break;
            case 0xbf:
                cycle += res_7_a();
                break;
            case 0xc0:
                cycle += set_0_b();
                break;
            case 0xc1:
                cycle += set_0_c();
                break;
            case 0xc2:
                cycle += set_0_d();
                break;
            case 0xc3:
                cycle += set_0_e();
                break;
            case 0xc4:
                cycle += set_0_h();
                break;
            case 0xc5:
                cycle += set_0_l();
                break;
            case 0xc6:
                cycle += set_0_addr_hl();
                break;
            case 0xc7:
                cycle += set_0_a();
                break;
            case 0xc8:
                cycle += set_1_b();
                break;
            case 0xc9:
                cycle += set_1_c();
                break;
            case 0xca:
                cycle += set_1_d();
                break;
            case 0xcb:
                cycle += set_1_e();
                break;
            case 0xcc:
                cycle += set_1_h();
                break;
            case 0xcd:
                cycle += set_1_l();
                break;
            case 0xce:
                cycle += set_1_addr_hl();
                break;
            case 0xcf:
                cycle += set_1_a();
                break;
            case 0xd0:
                cycle += set_2_b();
                break;
            case 0xd1:
                cycle += set_2_c();
                break;
            case 0xd2:
                cycle += set_2_d();
                break;
            case 0xd3:
                cycle += set_2_e();
                break;
            case 0xd4:
                cycle += set_2_h();
                break;
            case 0xd5:
                cycle += set_2_l();
                break;
            case 0xd6:
                cycle += set_2_addr_hl();
                break;
            case 0xd7:
                cycle += set_2_a();
                break;
            case 0xd8:
                cycle += set_3_b();
                break;
            case 0xd9:
                cycle += set_3_c();
                break;
            case 0xda:
                cycle += set_3_d();
                break;
            case 0xdb:
                cycle += set_3_e();
                break;
            case 0xdc:
                cycle += set_3_h();
                break;
            case 0xdd:
                cycle += set_3_l();
                break;
            case 0xde:
                cycle += set_3_addr_hl();
                break;
            case 0xdf:
                cycle += set_3_a();
                break;
            case 0xe0:
                cycle += set_4_b();
                break;
            case 0xe1:
                cycle += set_4_c();
                break;
            case 0xe2:
                cycle += set_4_d();
                break;
            case 0xe3:
                cycle += set_4_e();
                break;
            case 0xe4:
                cycle += set_4_h();
                break;
            case 0xe5:
                cycle += set_4_l();
                break;
            case 0xe6:
                cycle += set_4_addr_hl();
                break;
            case 0xe7:
                cycle += set_4_a();
                break;
            case 0xe8:
                cycle += set_5_b();
                break;
            case 0xe9:
                cycle += set_5_c();
                break;
            case 0xea:
                cycle += set_5_d();
                break;
            case 0xeb:
                cycle += set_5_e();
                break;
            case 0xec:
                cycle += set_5_h();
                break;
            case 0xed:
                cycle += set_5_l();
                break;
            case 0xee:
                cycle += set_5_addr_hl();
                break;
            case 0xef:
                cycle += set_5_a();
                break;
            case 0xf0:
                cycle += set_6_b();
                break;
            case 0xf1:
                cycle += set_6_c();
                break;
            case 0xf2:
                cycle += set_6_d();
                break;
            case 0xf3:
                cycle += set_6_e();
                break;
            case 0xf4:
                cycle += set_6_h();
                break;
            case 0xf5:
                cycle += set_6_l();
                break;
            case 0xf6:
                cycle += set_6_addr_hl();
                break;
            case 0xf7:
                cycle += set_6_a();
                break;
            case 0xf8:
                cycle += set_7_b();
                break;
            case 0xf9:
                cycle += set_7_c();
                break;
            case 0xfa:
                cycle += set_7_d();
                break;
            case 0xfb:
                cycle += set_7_e();
                break;
            case 0xfc:
                cycle += set_7_h();
                break;
            case 0xfd:
                cycle += set_7_l();
                break;
            case 0xfe:
                cycle += set_7_addr_hl();
                break;
            case 0xff:
                cycle += set_7_a();
                break;
            }
            break;
        }
    }
    --cycle;
}

std::uint8_t cpu::ld_b_b()
{
    b = b;

    return 4;
}

std::uint8_t cpu::ld_b_c()
{
    b = c;

    return 4;
}

std::uint8_t cpu::ld_b_d()
{
    b = d;

    return 4;
}

std::uint8_t cpu::ld_b_e()
{
    b = e;

    return 4;
}

std::uint8_t cpu::ld_b_h()
{
    b = h;

    return 4;
}

std::uint8_t cpu::ld_b_l()
{
    b = l;

    return 4;
}

std::uint8_t cpu::ld_b_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    b = read(ahl);

    return 8;
}

std::uint8_t cpu::ld_b_a()
{
    b = a;

    return 4;
}

std::uint8_t cpu::ld_c_b()
{
    c = b;

    return 4;
}

std::uint8_t cpu::ld_c_c()
{
    c = c;

    return 4;
}

std::uint8_t cpu::ld_c_d()
{
    c = d;

    return 4;
}

std::uint8_t cpu::ld_c_e()
{
    c = e;

    return 4;
}

std::uint8_t cpu::ld_c_h()
{
    c = h;

    return 4;
}

std::uint8_t cpu::ld_c_l()
{
    c = l;

    return 4;
}

std::uint8_t cpu::ld_c_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    c = read(ahl);

    return 8;
}

std::uint8_t cpu::ld_c_a()
{
    c = a;

    return 4;
}

std::uint8_t cpu::ld_d_b()
{
    d = b;

    return 4;
}

std::uint8_t cpu::ld_d_c()
{
    d = c;

    return 4;
}

std::uint8_t cpu::ld_d_d()
{
    d = d;

    return 4;
}

std::uint8_t cpu::ld_d_e()
{
    d = e;

    return 4;
}

std::uint8_t cpu::ld_d_h()
{
    d = h;

    return 4;
}

std::uint8_t cpu::ld_d_l()
{
    d = l;

    return 4;
}

std::uint8_t cpu::ld_d_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    d = read(ahl);

    return 8;
}

std::uint8_t cpu::ld_d_a()
{
    d = a;

    return 4;
}

std::uint8_t cpu::ld_e_b()
{
    e = b;

    return 4;
}

std::uint8_t cpu::ld_e_c()
{
    e = c;

    return 4;
}

std::uint8_t cpu::ld_e_d()
{
    e = d;

    return 4;
}

std::uint8_t cpu::ld_e_e()
{
    e = e;

    return 4;
}

std::uint8_t cpu::ld_e_h()
{
    e = h;

    return 4;
}

std::uint8_t cpu::ld_e_l()
{
    e = l;

    return 4;
}

std::uint8_t cpu::ld_e_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    e = read(ahl);

    return 8;
}

std::uint8_t cpu::ld_e_a()
{
    e = a;

    return 4;
}

std::uint8_t cpu::ld_h_b()
{
    h = b;

    return 4;
}

std::uint8_t cpu::ld_h_c()
{
    h = c;

    return 4;
}

std::uint8_t cpu::ld_h_d()
{
    h = d;

    return 4;
}

std::uint8_t cpu::ld_h_e()
{
    h = e;

    return 4;
}

std::uint8_t cpu::ld_h_h()
{
    h = h;

    return 4;
}

std::uint8_t cpu::ld_h_l()
{
    h = l;

    return 4;
}

std::uint8_t cpu::ld_h_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    h = read(ahl);

    return 8;
}

std::uint8_t cpu::ld_h_a()
{
    h = a;

    return 4;
}

std::uint8_t cpu::ld_l_b()
{
    l = b;

    return 4;
}

std::uint8_t cpu::ld_l_c()
{
    l = c;

    return 4;
}

std::uint8_t cpu::ld_l_d()
{
    l = d;

    return 4;
}

std::uint8_t cpu::ld_l_e()
{
    l = e;

    return 4;
}

std::uint8_t cpu::ld_l_h()
{
    l = h;

    return 4;
}

std::uint8_t cpu::ld_l_l()
{
    l = l;

    return 4;
}

std::uint8_t cpu::ld_l_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    l = read(ahl);

    return 8;
}

std::uint8_t cpu::ld_l_a()
{
    l = a;

    return 4;
}

std::uint8_t cpu::ld_addr_hl_b()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, b);

    return 8;
}

std::uint8_t cpu::ld_addr_hl_c()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, c);

    return 8;
}

std::uint8_t cpu::ld_addr_hl_d()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, d);

    return 8;
}

std::uint8_t cpu::ld_addr_hl_e()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, e);

    return 8;
}

std::uint8_t cpu::ld_addr_hl_h()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, h);

    return 8;
}

std::uint8_t cpu::ld_addr_hl_l()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, l);

    return 8;
}

std::uint8_t cpu::ld_addr_hl_a()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, a);

    return 8;
}

std::uint8_t cpu::ld_a_b()
{
    a = b;

    return 4;
}

std::uint8_t cpu::ld_a_c()
{
    a = c;

    return 4;
}

std::uint8_t cpu::ld_a_d()
{
    a = d;

    return 4;
}

std::uint8_t cpu::ld_a_e()
{
    a = e;

    return 4;
}

std::uint8_t cpu::ld_a_h()
{
    a = h;

    return 4;
}

std::uint8_t cpu::ld_a_l()
{
    a = l;

    return 4;
}

std::uint8_t cpu::ld_a_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    a = read(ahl);

    return 8;
}

std::uint8_t cpu::ld_a_a()
{
    a = a;

    return 4;
}

std::uint8_t cpu::ld_b_d8()
{
    b = read(pc);
    ++pc;

    return 8;
}

std::uint8_t cpu::ld_c_d8()
{
    c = read(pc);
    ++pc;

    return 8;
}

std::uint8_t cpu::ld_d_d8()
{
    d = read(pc);
    ++pc;

    return 8;
}

std::uint8_t cpu::ld_e_d8()
{
    e = read(pc);
    ++pc;

    return 8;
}

std::uint8_t cpu::ld_h_d8()
{
    h = read(pc);
    ++pc;

    return 8;
}

std::uint8_t cpu::ld_l_d8()
{
    l = read(pc);
    ++pc;

    return 8;
}

std::uint8_t cpu::ld_addr_hl_d8()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, read(pc));
    ++pc;

    return 12;
}

std::uint8_t cpu::ld_a_d8()
{
    a = read(pc);
    ++pc;

    return 8;
}

std::uint8_t cpu::ld_addr_bc_a()
{
    std::uint16_t abc = (b << 8) | c;
    write(abc, a);

    return 8;
}

std::uint8_t cpu::ld_addr_de_a()
{
    std::uint16_t ade = (d << 8) | e;
    write(ade, a);

    return 8;
}

std::uint8_t cpu::ldi_addr_hl_a()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, a);
    ++ahl;
    h = ahl >> 8;
    l = ahl;

    return 8;
}

std::uint8_t cpu::ldd_addr_hl_a()
{
    std::uint16_t ahl = (h << 8) | l;
    write(ahl, a);
    --ahl;
    h = ahl >> 8;
    l = ahl;

    return 8;
}

std::uint8_t cpu::ld_a_addr_bc()
{
    std::uint16_t abc = (b << 8) | c;
    a = read(abc);

    return 8;
}

std::uint8_t cpu::ld_a_addr_de()
{
    std::uint16_t ade = (d << 8) | e;
    a = read(ade);

    return 8;
}

std::uint8_t cpu::ldi_a_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    a = read(ahl);
    ++ahl;
    h = ahl >> 8;
    l = ahl;

    return 8;
}

std::uint8_t cpu::ldd_a_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    a = read(ahl);
    --ahl;
    h = ahl >> 8;
    l = ahl;

    return 8;
}

std::uint8_t cpu::ldh_a8_a()
{
    write(0xff00 + read(pc), a);
    ++pc;

    return 12;
}

std::uint8_t cpu::ldh_a_a8()
{
    a = read(0xff00 + read(pc));
    ++pc;

    return 12;
}

std::uint8_t cpu::ld_addr_c_a()
{
    write(0xff00 + c, a);

    return 8;
}

std::uint8_t cpu::ld_a_addr_c()
{
    a = read(0xff00 + c);

    return 8;
}

std::uint8_t cpu::ld_a16_a()
{
    std::uint16_t a16 = read(pc) | (read(++pc) << 8);
    ++pc;
    write(a16, a);

    return 16;
}

std::uint8_t cpu::ld_a_a16()
{
    std::uint16_t a16 = read(pc) | (read(++pc) << 8);
    ++pc;
    a = read(a16);

    return 16;
}

std::uint8_t cpu::ld_bc_d16()
{
    c = read(pc);
    b = read(++pc);
    ++pc;

    return 12;
}

std::uint8_t cpu::ld_de_d16()
{
    e = read(pc);
    d = read(++pc);
    ++pc;

    return 12;
}

std::uint8_t cpu::ld_hl_d16()
{
    l = read(pc);
    h = read(++pc);
    ++pc;

    return 12;
}

std::uint8_t cpu::ld_sp_d16()
{
    std::uint16_t d16 = read(pc) | (read(++pc) << 8);
    ++pc;
    sp = d16;

    return 12;
}

std::uint8_t cpu::ld_a16_sp()
{
    std::uint16_t a16 = read(pc) | (read(++pc) << 8);
    ++pc;
    std::uint8_t high = sp >> 8;
    std::uint8_t low = sp;
    write(a16, low);
    write(++a16, high);

    return 20;
}

std::uint8_t cpu::ld_sp_hl()
{
    std::uint16_t hl = (h << 8) | l;
    sp = hl;

    return 8;
}

std::uint8_t cpu::ld_hl_sp_plus_r8()
{
    std::uint16_t hl = (h << 8) | l;
    std::uint8_t r8 = read(pc);
    ++pc;
    f = f & ~(1 << 7);
    f = f & ~(1 << 6);
    if (((sp & 0xf) + (r8 & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (((sp & 0xff) + r8) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    hl = sp + (std::int8_t)r8;
    h = hl >> 8;
    l = hl;

    return 12;
}

std::uint8_t cpu::pop_bc()
{
    c = read(sp);
    ++sp;
    b = read(sp);
    ++sp;

    return 12;
}

std::uint8_t cpu::pop_de()
{
    e = read(sp);
    ++sp;
    d = read(sp);
    ++sp;

    return 12;
}

std::uint8_t cpu::pop_hl()
{
    l = read(sp);
    ++sp;
    h = read(sp);
    ++sp;

    return 12;
}

std::uint8_t cpu::pop_af()
{
    f = read(sp);
    f = f & 0xf0;
    ++sp;
    a = read(sp);
    ++sp;

    return 12;
}

std::uint8_t cpu::push_bc()
{
    --sp;
    write(sp, b);
    --sp;
    write(sp, c);

    return 16;
}

std::uint8_t cpu::push_de()
{
    --sp;
    write(sp, d);
    --sp;
    write(sp, e);

    return 16;
}

std::uint8_t cpu::push_hl()
{
    --sp;
    write(sp, h);
    --sp;
    write(sp, l);

    return 16;
}

std::uint8_t cpu::push_af()
{
    --sp;
    write(sp, a);
    --sp;
    write(sp, f);

    return 16;
}

std::uint8_t cpu::inc_b()
{
    f = f & ~(1 << 6);
    if ((b & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++b;
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::inc_c()
{
    f = f & ~(1 << 6);
    if ((c & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++c;
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::inc_d()
{
    f = f & ~(1 << 6);
    if ((d & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++d;
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::inc_e()
{
    f = f & ~(1 << 6);
    if ((e & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++e;
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::inc_h()
{
    f = f & ~(1 << 6);
    if ((h & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++h;
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::inc_l()
{
    f = f & ~(1 << 6);
    if ((l & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++l;
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::inc_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    f = f & ~(1 << 6);
    if ((memory & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++memory;
    write(ahl, memory);
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 12;
}

std::uint8_t cpu::inc_a()
{
    f = f & ~(1 << 6);
    if ((a & 0xf) + (1 & 0xf) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    ++a;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::dec_b()
{
    f = f | (1 << 6);
    if ((b & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --b;
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::dec_c()
{
    f = f | (1 << 6);
    if ((c & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --c;
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::dec_d()
{
    f = f | (1 << 6);
    if ((d & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --d;
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::dec_e()
{
    f = f | (1 << 6);
    if ((e & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --e;
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::dec_h()
{
    f = f | (1 << 6);
    if ((h & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --h;
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::dec_l()
{
    f = f | (1 << 6);
    if ((l & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --l;
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::dec_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    f = f | (1 << 6);
    if ((memory & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --memory;
    write(ahl, memory);
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 12;
}

std::uint8_t cpu::dec_a()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (1 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    --a;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_b()
{
    f = f & ~(1 << 6);
    if (((a & 0xf) + (b & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + b) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += b;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_c()
{
    f = f & ~(1 << 6);
    if (((a & 0xf) + (c & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + c) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += c;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_d()
{
    f = f & ~(1 << 6);
    if (((a & 0xf) + (d & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + d) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += d;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_e()
{
    f = f & ~(1 << 6);
    if (((a & 0xf) + (e & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + e) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += e;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_h()
{
    f = f & ~(1 << 6);
    if (((a & 0xf) + (h & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + h) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += h;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_l()
{
    f = f & ~(1 << 6);
    if (((a & 0xf) + (l & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + l) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += l;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (memory & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + memory) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += memory;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::add_a_a()
{
    f = f & ~(1 << 6);
    if (((a & 0xf) + (a & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + a) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += a;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::adc_a_b()
{
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (b & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + b + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += b + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::adc_a_c()
{
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (c & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + c + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += c + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::adc_a_d()
{
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (d & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + d + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += d + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::adc_a_e()
{
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (e & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + e + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += e + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::adc_a_h()
{
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (h & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + h + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += h + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::adc_a_l()
{
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (l & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + l + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += l + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::adc_a_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (memory & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + memory + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += memory + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::adc_a_a()
{
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (a & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + a + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += a + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sub_b()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (b & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < b)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= b;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sub_c()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (c & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < c)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= c;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sub_d()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (d & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < d)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= d;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sub_e()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (e & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < e)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= e;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sub_h()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (h & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < h)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= h;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sub_l()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (l & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < l)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= l;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sub_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    f = f | (1 << 6);
    if ((a & 0xf) < (memory & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < memory)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= memory;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sub_a()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (a & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < a)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= a;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sbc_a_b()
{
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((b & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (b + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (b + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sbc_a_c()
{
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((c & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (c + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (c + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sbc_a_d()
{
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((d & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (d + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (d + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sbc_a_e()
{
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((e & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (e + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (e + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sbc_a_h()
{
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((h & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (h + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (h + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sbc_a_l()
{
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((l & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (l + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (l + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::sbc_a_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((memory & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (memory + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (memory + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sbc_a_a()
{
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((a & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (a + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (a + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::and_b()
{
    a = a & b;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::and_c()
{
    a = a & c;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::and_d()
{
    a = a & d;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::and_e()
{
    a = a & e;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::and_h()
{
    a = a & h;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::and_l()
{
    a = a & l;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::and_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    a = a & memory;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::and_a()
{
    a = a & a;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::xor_b()
{
    a = a ^ b;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::xor_c()
{
    a = a ^ c;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::xor_d()
{
    a = a ^ d;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::xor_e()
{
    a = a ^ e;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::xor_h()
{
    a = a ^ h;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::xor_l()
{
    a = a ^ l;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::xor_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    a = a ^ memory;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::xor_a()
{
    a = a ^ a;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::or_b()
{
    a = a | b;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::or_c()
{
    a = a | c;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::or_d()
{
    a = a | d;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::or_e()
{
    a = a | e;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::or_h()
{
    a = a | h;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::or_l()
{
    a = a | l;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::or_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    a = a | memory;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::or_a()
{
    a = a | a;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 4;
}

std::uint8_t cpu::cp_b()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (b & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < b)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::cp_c()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (c & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < c)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::cp_d()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (d & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < d)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::cp_e()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (e & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < e)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::cp_h()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (h & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < h)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::cp_l()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (l & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < l)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::cp_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    f = f | (1 << 6);
    if ((a & 0xf) < (memory & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < memory)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::cp_a()
{
    f = f | (1 << 6);
    if ((a & 0xf) < (a & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < a)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::add_a_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    f = f & ~(1 << 6);
    if (((a & 0xf) + (d8 & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + d8) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += d8;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::adc_a_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    bool carry_flag = f & (1 << 4);
    f = f & ~(1 << 6);
    if (((a & 0xf) + (d8 & 0xf) + carry_flag) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((a + d8 + carry_flag) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a += d8 + carry_flag;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sub_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    f = f | (1 << 6);
    if ((a & 0xf) < (d8 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < d8)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= d8;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sbc_a_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    bool carry_flag = f & (1 << 4);
    f = f | (1 << 6);
    if ((a & 0xf) < ((d8 & 0xf) + carry_flag))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < (d8 + carry_flag))
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a -= (d8 + carry_flag);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::and_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    a = a & d8;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);
    f = f & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::xor_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    a = a ^ d8;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::or_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    a = a | d8;
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::cp_d8()
{
    std::uint8_t d8 = read(pc);
    ++pc;
    f = f | (1 << 6);
    if ((a & 0xf) < (d8 & 0xf))
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (a < d8)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (a - d8 == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::daa()
{
    bool half_carry_flag = f & (1 << 5);
    bool carry_flag = f & (1 << 4);
    bool subtraction = f & (1 << 6);
    if (!subtraction)
    {
        if (a > 0x99 || carry_flag)
        {
            a += 0x60;
            f = f | (1 << 4);
        }
        if ((a & 0xf) > 0x09 || half_carry_flag)
        {
            a += 0x06;
        }
    }
    else
    {
        if (carry_flag)
        {
            a -= 0x60;
        }
        if (half_carry_flag)
        {
            a -= 0x06;
        }
    }
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 5);

    return 4;
}

std::uint8_t cpu::cpl()
{
    a = ~a;
    f = f | (1 << 6);
    f = f | (1 << 5);

    return 4;
}

std::uint8_t cpu::scf()
{
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f | (1 << 4);

    return 4;
}

std::uint8_t cpu::ccf()
{
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f ^ (1 << 4);

    return 4;
}

std::uint8_t cpu::inc_bc()
{
    std::uint16_t bc = (b << 8) | c;
    b = ++bc >> 8;
    c = bc;

    return 8;
}

std::uint8_t cpu::inc_de()
{
    std::uint16_t de = (d << 8) | e;
    d = ++de >> 8;
    e = de;

    return 8;
}

std::uint8_t cpu::inc_hl()
{
    std::uint16_t hl = (h << 8) | l;
    h = ++hl >> 8;
    l = hl;

    return 8;
}

std::uint8_t cpu::inc_sp()
{
    ++sp;

    return 8;
}

std::uint8_t cpu::dec_bc()
{
    std::uint16_t bc = (b << 8) | c;
    b = --bc >> 8;
    c = bc;

    return 8;
}

std::uint8_t cpu::dec_de()
{
    std::uint16_t de = (d << 8) | e;
    d = --de >> 8;
    e = de;

    return 8;
}

std::uint8_t cpu::dec_hl()
{
    std::uint16_t hl = (h << 8) | l;
    h = --hl >> 8;
    l = hl;

    return 8;
}

std::uint8_t cpu::dec_sp()
{
    --sp;

    return 8;
}

std::uint8_t cpu::add_hl_bc()
{
    std::uint16_t hl = (h << 8) | l;
    std::uint16_t bc = (b << 8) | c;
    f = f & ~(1 << 6);
    if (((hl & 0xfff) + (bc & 0xfff)) > 0xfff)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((hl + bc) > 0xffff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    hl += bc;
    h = (hl >> 8);
    l = hl;

    return 8;
}

std::uint8_t cpu::add_hl_de()
{
    std::uint16_t hl = (h << 8) | l;
    std::uint16_t de = (d << 8) | e;
    f = f & ~(1 << 6);
    if (((hl & 0xfff) + (de & 0xfff)) > 0xfff)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((hl + de) > 0xffff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    hl += de;
    h = (hl >> 8);
    l = hl;

    return 8;
}

std::uint8_t cpu::add_hl_hl()
{
    std::uint16_t hl = (h << 8) | l;
    f = f & ~(1 << 6);
    if (((hl & 0xfff) + (hl & 0xfff)) > 0xfff)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((hl + hl) > 0xffff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    hl += hl;
    h = (hl >> 8);
    l = hl;

    return 8;
}

std::uint8_t cpu::add_hl_sp()
{
    std::uint16_t hl = (h << 8) | l;
    f = f & ~(1 << 6);
    if (((hl & 0xfff) + (sp & 0xfff)) > 0xfff)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if ((hl + sp) > 0xffff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    hl += sp;
    h = (hl >> 8);
    l = hl;

    return 8;
}

std::uint8_t cpu::add_sp_r8()
{
    std::uint8_t r8 = read(pc);
    ++pc;
    f = f & ~(1 << 7);
    f = f & ~(1 << 6);
    if (((sp & 0xf) + (r8 & 0xf)) > 0xf)
    {
        f = f | (1 << 5);
    }
    else
    {
        f = f & ~(1 << 5);
    }
    if (((sp & 0xff) + r8) > 0xff)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }

    sp = sp + (std::int8_t)r8;

    return 16;
}

std::uint8_t cpu::jr_r8()
{
    std::int8_t r8 = read(pc);
    ++pc;
    pc += r8;

    return 12;
}

std::uint8_t cpu::jr_z_r8()
{
    bool zero_flag = f & (1 << 7);
    if (zero_flag)
    {
        std::int8_t r8 = read(pc);
        ++pc;
        pc += r8;
        return 12;
    }
    ++pc;

    return 8;
}

std::uint8_t cpu::jr_nz_r8()
{
    bool zero_flag = f & (1 << 7);
    if (!zero_flag)
    {
        std::int8_t r8 = read(pc);
        ++pc;
        pc += r8;
        return 12;
    }
    ++pc;

    return 8;
}

std::uint8_t cpu::jr_c_r8()
{
    bool carry_flag = f & (1 << 4);
    if (carry_flag)
    {
        std::int8_t r8 = read(pc);
        ++pc;
        pc += r8;
        return 12;
    }
    ++pc;

    return 8;
}

std::uint8_t cpu::jr_nc_r8()
{
    bool carry_flag = f & (1 << 4);
    if (!carry_flag)
    {
        std::int8_t r8 = read(pc);
        ++pc;
        pc += r8;
        return 12;
    }
    ++pc;

    return 8;
}

std::uint8_t cpu::jp_a16()
{
    std::uint16_t a16 = read(pc) | (read(++pc) << 8);
    ++pc;
    pc = a16;

    return 16;
}

std::uint8_t cpu::jp_z_a16()
{
    bool zero_flag = f & (1 << 7);
    if (zero_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        pc = a16;
        return 16;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::jp_nz_a16()
{
    bool zero_flag = f & (1 << 7);
    if (!zero_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        pc = a16;
        return 16;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::jp_c_a16()
{
    bool carry_flag = f & (1 << 4);
    if (carry_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        pc = a16;
        return 16;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::jp_nc_a16()
{
    bool carry_flag = f & (1 << 4);
    if (!carry_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        pc = a16;
        return 16;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::ret()
{
    pc = read(sp) | (read(++sp) << 8);
    ++sp;

    return 16;
}

std::uint8_t cpu::ret_z()
{
    bool zero_flag = f & (1 << 7);
    if (zero_flag)
    {
        pc = read(sp) | (read(++sp) << 8);
        ++sp;
        return 20;
    }

    return 8;
}

std::uint8_t cpu::ret_nz()
{
    bool zero_flag = f & (1 << 7);
    if (!zero_flag)
    {
        pc = read(sp) | (read(++sp) << 8);
        ++sp;
        return 20;
    }

    return 8;
}

std::uint8_t cpu::ret_c()
{
    bool carry_flag = f & (1 << 4);
    if (carry_flag)
    {
        pc = read(sp) | (read(++sp) << 8);
        ++sp;
        return 20;
    }

    return 8;
}

std::uint8_t cpu::ret_nc()
{
    bool carry_flag = f & (1 << 4);
    if (!carry_flag)
    {
        pc = read(sp) | (read(++sp) << 8);
        ++sp;
        return 20;
    }

    return 8;
}

std::uint8_t cpu::reti()
{
    pc = read(sp) | (read(++sp) << 8);
    ++sp;
    ime_flag = true;

    return 16;
}

std::uint8_t cpu::jp_hl()
{
    std::uint16_t hl = (h << 8) | l;
    pc = hl;

    return 4;
}

std::uint8_t cpu::call_a16()
{
    std::uint16_t a16 = read(pc) | (read(++pc) << 8);
    ++pc;
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    pc = a16;

    return 24;
}

std::uint8_t cpu::call_z_a16()
{
    bool zero_flag = f & (1 << 7);
    if (zero_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        std::uint8_t high = pc >> 8;
        std::uint8_t low = pc;
        --sp;
        write(sp, high);
        --sp;
        write(sp, low);
        pc = a16;
        return 24;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::call_nz_a16()
{
    bool zero_flag = f & (1 << 7);
    if (!zero_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        std::uint8_t high = pc >> 8;
        std::uint8_t low = pc;
        --sp;
        write(sp, high);
        --sp;
        write(sp, low);
        pc = a16;
        return 24;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::call_c_a16()
{
    bool carry_flag = f & (1 << 4);
    if (carry_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        std::uint8_t high = pc >> 8;
        std::uint8_t low = pc;
        --sp;
        write(sp, high);
        --sp;
        write(sp, low);
        pc = a16;
        return 24;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::call_nc_a16()
{
    bool carry_flag = f & (1 << 4);
    if (!carry_flag)
    {
        std::uint16_t a16 = read(pc) | (read(++pc) << 8);
        ++pc;
        std::uint8_t high = pc >> 8;
        std::uint8_t low = pc;
        --sp;
        write(sp, high);
        --sp;
        write(sp, low);
        pc = a16;
        return 24;
    }
    ++pc;
    ++pc;

    return 12;
}

std::uint8_t cpu::rst_00h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x00;
    pc = address;

    return 16;
}

std::uint8_t cpu::rst_08h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x08;
    pc = address;

    return 16;
}

std::uint8_t cpu::rst_10h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x10;
    pc = address;

    return 16;
}

std::uint8_t cpu::rst_18h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x18;
    pc = address;

    return 16;
}

std::uint8_t cpu::rst_20h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x20;
    pc = address;

    return 16;
}

std::uint8_t cpu::rst_28h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x28;
    pc = address;

    return 16;
}

std::uint8_t cpu::rst_30h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x30;
    pc = address;

    return 16;
}

std::uint8_t cpu::rst_38h()
{
    std::uint8_t high = pc >> 8;
    std::uint8_t low = pc;
    --sp;
    write(sp, high);
    --sp;
    write(sp, low);
    std::uint16_t address = 0x0000 + 0x38;
    pc = address;

    return 16;
}

std::uint8_t cpu::rlca()
{
    bool bit_7_set = a & (1 << 7);
    a = a << 1;
    f = f & ~(1 << 7);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        a = a | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        a = a & ~(1 << 0);
    }

    return 4;
}

std::uint8_t cpu::rla()
{
    bool bit_7_set = a & (1 << 7);
    bool carry_flag = f & (1 << 4);
    a = a << 1;
    f = f & ~(1 << 7);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        a = a | (1 << 0);
    }
    else
    {
        a = a & ~(1 << 0);
    }

    return 4;
}

std::uint8_t cpu::rrca()
{
    bool bit_0_set = a & (1 << 0);
    a = a >> 1;
    f = f & ~(1 << 7);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        a = a | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        a = a & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::rra()
{
    bool bit_0_set = a & (1 << 0);
    bool carry_flag = f & (1 << 4);
    a = a >> 1;
    f = f & ~(1 << 7);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        a = a | (1 << 7);
    }
    else
    {
        a = a & ~(1 << 7);
    }

    return 4;
}

std::uint8_t cpu::stop()
{
    return 4;
}

std::uint8_t cpu::halt()
{
    halted = true;

    return 0;
}

std::uint8_t cpu::ei()
{
    ime_flag = true;

    return 4;
}

std::uint8_t cpu::di()
{
    ime_flag = false;

    return 4;
}

std::uint8_t cpu::nop()
{
    return 4;
}

std::uint8_t cpu::rlc_b()
{
    bool bit_7_set = b & (1 << 7);
    b = b << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        b = b | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        b = b & ~(1 << 0);
    }
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rlc_c()
{
    bool bit_7_set = c & (1 << 7);
    c = c << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        c = c | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        c = c & ~(1 << 0);
    }
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rlc_d()
{
    bool bit_7_set = d & (1 << 7);
    d = d << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        d = d | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        d = d & ~(1 << 0);
    }
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rlc_e()
{
    bool bit_7_set = e & (1 << 7);
    e = e << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        e = e | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        e = e & ~(1 << 0);
    }
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rlc_h()
{
    bool bit_7_set = h & (1 << 7);
    h = h << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        h = h | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        h = h & ~(1 << 0);
    }
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rlc_l()
{
    bool bit_7_set = l & (1 << 7);
    l = l << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        l = l | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        l = l & ~(1 << 0);
    }
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rlc_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_7_set = memory & (1 << 7);
    memory = memory << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        memory = memory | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        memory = memory & ~(1 << 0);
    }
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::rlc_a()
{
    bool bit_7_set = a & (1 << 7);
    a = a << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
        a = a | (1 << 0);
    }
    else
    {
        f = f & ~(1 << 4);
        a = a & ~(1 << 0);
    }
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rrc_b()
{
    bool bit_0_set = b & (1 << 0);
    b = b >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        b = b | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        b = b & ~(1 << 7);
    }
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rrc_c()
{
    bool bit_0_set = c & (1 << 0);
    c = c >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        c = c | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        c = c & ~(1 << 7);
    }
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rrc_d()
{
    bool bit_0_set = d & (1 << 0);
    d = d >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        d = d | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        d = d & ~(1 << 7);
    }
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rrc_e()
{
    bool bit_0_set = e & (1 << 0);
    e = e >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        e = e | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        e = e & ~(1 << 7);
    }
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rrc_h()
{
    bool bit_0_set = h & (1 << 0);
    h = h >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        h = h | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        h = h & ~(1 << 7);
    }
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rrc_l()
{
    bool bit_0_set = l & (1 << 0);
    l = l >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        l = l | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        l = l & ~(1 << 7);
    }
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rrc_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_0_set = memory & (1 << 0);
    memory = memory >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        memory = memory | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        memory = memory & ~(1 << 7);
    }
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::rrc_a()
{
    bool bit_0_set = a & (1 << 0);
    a = a >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
        a = a | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 4);
        a = a & ~(1 << 7);
    }
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rl_b()
{
    bool bit_7_set = b & (1 << 7);
    bool carry_flag = f & (1 << 4);
    b = b << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        b = b | (1 << 0);
    }
    else
    {
        b = b & ~(1 << 0);
    }
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rl_c()
{
    bool bit_7_set = c & (1 << 7);
    bool carry_flag = f & (1 << 4);
    c = c << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        c = c | (1 << 0);
    }
    else
    {
        c = c & ~(1 << 0);
    }
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rl_d()
{
    bool bit_7_set = d & (1 << 7);
    bool carry_flag = f & (1 << 4);
    d = d << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        d = d | (1 << 0);
    }
    else
    {
        d = d & ~(1 << 0);
    }
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rl_e()
{
    bool bit_7_set = e & (1 << 7);
    bool carry_flag = f & (1 << 4);
    e = e << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        e = e | (1 << 0);
    }
    else
    {
        e = e & ~(1 << 0);
    }
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rl_h()
{
    bool bit_7_set = h & (1 << 7);
    bool carry_flag = f & (1 << 4);
    h = h << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        h = h | (1 << 0);
    }
    else
    {
        h = h & ~(1 << 0);
    }
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rl_l()
{
    bool bit_7_set = l & (1 << 7);
    bool carry_flag = f & (1 << 4);
    l = l << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        l = l | (1 << 0);
    }
    else
    {
        l = l & ~(1 << 0);
    }
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rl_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_7_set = memory & (1 << 7);
    bool carry_flag = f & (1 << 4);
    memory = memory << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        memory = memory | (1 << 0);
    }
    else
    {
        memory = memory & ~(1 << 0);
    }
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::rl_a()
{
    bool bit_7_set = a & (1 << 7);
    bool carry_flag = f & (1 << 4);
    a = a << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        a = a | (1 << 0);
    }
    else
    {
        a = a & ~(1 << 0);
    }
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rr_b()
{
    bool bit_0_set = b & (1 << 0);
    bool carry_flag = f & (1 << 4);
    b = b >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        b = b | (1 << 7);
    }
    else
    {
        b = b & ~(1 << 7);
    }
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rr_c()
{
    bool bit_0_set = c & (1 << 0);
    bool carry_flag = f & (1 << 4);
    c = c >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        c = c | (1 << 7);
    }
    else
    {
        c = c & ~(1 << 7);
    }
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rr_d()
{
    bool bit_0_set = d & (1 << 0);
    bool carry_flag = f & (1 << 4);
    d = d >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        d = d | (1 << 7);
    }
    else
    {
        d = d & ~(1 << 7);
    }
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rr_e()
{
    bool bit_0_set = e & (1 << 0);
    bool carry_flag = f & (1 << 4);
    e = e >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        e = e | (1 << 7);
    }
    else
    {
        e = e & ~(1 << 7);
    }
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rr_h()
{
    bool bit_0_set = h & (1 << 0);
    bool carry_flag = f & (1 << 4);
    h = h >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        h = h | (1 << 7);
    }
    else
    {
        h = h & ~(1 << 7);
    }
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rr_l()
{
    bool bit_0_set = l & (1 << 0);
    bool carry_flag = f & (1 << 4);
    l = l >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        l = l | (1 << 7);
    }
    else
    {
        l = l & ~(1 << 7);
    }
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::rr_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_0_set = memory & (1 << 0);
    bool carry_flag = f & (1 << 4);
    memory = memory >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        memory = memory | (1 << 7);
    }
    else
    {
        memory = memory & ~(1 << 7);
    }
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::rr_a()
{
    bool bit_0_set = a & (1 << 0);
    bool carry_flag = f & (1 << 4);
    a = a >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (carry_flag)
    {
        a = a | (1 << 7);
    }
    else
    {
        a = a & ~(1 << 7);
    }
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sla_b()
{
    bool bit_7_set = b & (1 << 7);
    b = b << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    b = b & ~(1 << 0);
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sla_c()
{
    bool bit_7_set = c & (1 << 7);
    c = c << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    c = c & ~(1 << 0);
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sla_d()
{
    bool bit_7_set = d & (1 << 7);
    d = d << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    d = d & ~(1 << 0);
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sla_e()
{
    bool bit_7_set = e & (1 << 7);
    e = e << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    e = e & ~(1 << 0);
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sla_h()
{
    bool bit_7_set = h & (1 << 7);
    h = h << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    h = h & ~(1 << 0);
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sla_l()
{
    bool bit_7_set = l & (1 << 7);
    l = l << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    l = l & ~(1 << 0);
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sla_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_7_set = memory & (1 << 7);
    memory = memory << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    memory = memory & ~(1 << 0);
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::sla_a()
{
    bool bit_7_set = a & (1 << 7);
    a = a << 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_7_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a = a & ~(1 << 0);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sra_b()
{
    bool bit_0_set = b & (1 << 0);
    bool bit_7_set = b & (1 << 7);
    b = b >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        b = b | (1 << 7);
    }
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sra_c()
{
    bool bit_0_set = c & (1 << 0);
    bool bit_7_set = c & (1 << 7);
    c = c >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        c = c | (1 << 7);
    }
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sra_d()
{
    bool bit_0_set = d & (1 << 0);
    bool bit_7_set = d & (1 << 7);
    d = d >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        d = d | (1 << 7);
    }
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sra_e()
{
    bool bit_0_set = e & (1 << 0);
    bool bit_7_set = e & (1 << 7);
    e = e >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        e = e | (1 << 7);
    }
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sra_h()
{
    bool bit_0_set = h & (1 << 0);
    bool bit_7_set = h & (1 << 7);
    h = h >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        h = h | (1 << 7);
    }
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sra_l()
{
    bool bit_0_set = l & (1 << 0);
    bool bit_7_set = l & (1 << 7);
    l = l >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        l = l | (1 << 7);
    }
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::sra_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_0_set = memory & (1 << 0);
    bool bit_7_set = memory & (1 << 7);
    memory = memory >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        memory = memory | (1 << 7);
    }
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::sra_a()
{
    bool bit_0_set = a & (1 << 0);
    bool bit_7_set = a & (1 << 7);
    a = a >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    if (bit_7_set)
    {
        a = a | (1 << 7);
    }
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::swap_b()
{
    b = (b << 4) | (b >> 4);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::swap_c()
{
    c = ((c & 0xf) << 4) | (c >> 4);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::swap_d()
{
    d = ((d & 0xf) << 4) | (d >> 4);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::swap_e()
{
    e = ((e & 0xf) << 4) | (e >> 4);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::swap_h()
{
    h = ((h & 0xf) << 4) | (h >> 4);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::swap_l()
{
    l = ((l & 0xf) << 4) | (l >> 4);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::swap_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = ((memory & 0xf) << 4) | (memory >> 4);
    write(ahl, memory);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 16;
}

std::uint8_t cpu::swap_a()
{
    a = ((a & 0xf) << 4) | (a >> 4);
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    f = f & ~(1 << 4);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::srl_b()
{
    bool bit_0_set = b & (1 << 0);
    b = b >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    b = b & ~(1 << 7);
    if (b == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::srl_c()
{
    bool bit_0_set = c & (1 << 0);
    c = c >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    c = c & ~(1 << 7);
    if (c == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::srl_d()
{
    bool bit_0_set = d & (1 << 0);
    d = d >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    d = d & ~(1 << 7);
    if (d == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::srl_e()
{
    bool bit_0_set = e & (1 << 0);
    e = e >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    e = e & ~(1 << 7);
    if (e == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::srl_h()
{
    bool bit_0_set = h & (1 << 0);
    h = h >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    h = h & ~(1 << 7);
    if (h == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::srl_l()
{
    bool bit_0_set = l & (1 << 0);
    l = l >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    l = l & ~(1 << 7);
    if (l == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::srl_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_0_set = memory & (1 << 0);
    memory = memory >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    memory = memory & ~(1 << 7);
    if (memory == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::srl_a()
{
    bool bit_0_set = a & (1 << 0);
    a = a >> 1;
    f = f & ~(1 << 6);
    f = f & ~(1 << 5);
    if (bit_0_set)
    {
        f = f | (1 << 4);
    }
    else
    {
        f = f & ~(1 << 4);
    }
    a = a & ~(1 << 7);
    if (a == 0)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }

    return 8;
}

std::uint8_t cpu::bit_0_b()
{
    bool bit_set = b & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_0_c()
{
    bool bit_set = c & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_0_d()
{
    bool bit_set = d & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_0_e()
{
    bool bit_set = e & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_0_h()
{
    bool bit_set = h & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_0_l()
{
    bool bit_set = l & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_0_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_0_a()
{
    bool bit_set = a & (1 << 0);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_1_b()
{
    bool bit_set = b & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_1_c()
{
    bool bit_set = c & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_1_d()
{
    bool bit_set = d & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_1_e()
{
    bool bit_set = e & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_1_h()
{
    bool bit_set = h & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_1_l()
{
    bool bit_set = l & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_1_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_1_a()
{
    bool bit_set = a & (1 << 1);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_2_b()
{
    bool bit_set = b & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_2_c()
{
    bool bit_set = c & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_2_d()
{
    bool bit_set = d & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_2_e()
{
    bool bit_set = e & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_2_h()
{
    bool bit_set = h & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_2_l()
{
    bool bit_set = l & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_2_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_2_a()
{
    bool bit_set = a & (1 << 2);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_3_b()
{
    bool bit_set = b & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_3_c()
{
    bool bit_set = c & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_3_d()
{
    bool bit_set = d & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_3_e()
{
    bool bit_set = e & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_3_h()
{
    bool bit_set = h & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_3_l()
{
    bool bit_set = l & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_3_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_3_a()
{
    bool bit_set = a & (1 << 3);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_4_b()
{
    bool bit_set = b & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_4_c()
{
    bool bit_set = c & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_4_d()
{
    bool bit_set = d & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_4_e()
{
    bool bit_set = e & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_4_h()
{
    bool bit_set = h & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_4_l()
{
    bool bit_set = l & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_4_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_4_a()
{
    bool bit_set = a & (1 << 4);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_5_b()
{
    bool bit_set = b & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_5_c()
{
    bool bit_set = c & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_5_d()
{
    bool bit_set = d & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_5_e()
{
    bool bit_set = e & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_5_h()
{
    bool bit_set = h & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_5_l()
{
    bool bit_set = l & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_5_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_5_a()
{
    bool bit_set = a & (1 << 5);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_6_b()
{
    bool bit_set = b & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_6_c()
{
    bool bit_set = c & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_6_d()
{
    bool bit_set = d & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_6_e()
{
    bool bit_set = e & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_6_h()
{
    bool bit_set = h & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_6_l()
{
    bool bit_set = l & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_6_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_6_a()
{
    bool bit_set = a & (1 << 6);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_7_b()
{
    bool bit_set = b & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_7_c()
{
    bool bit_set = c & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_7_d()
{
    bool bit_set = d & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_7_e()
{
    bool bit_set = e & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_7_h()
{
    bool bit_set = h & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_7_l()
{
    bool bit_set = l & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::bit_7_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    bool bit_set = memory & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 12;
}

std::uint8_t cpu::bit_7_a()
{
    bool bit_set = a & (1 << 7);
    if (!bit_set)
    {
        f = f | (1 << 7);
    }
    else
    {
        f = f & ~(1 << 7);
    }
    f = f & ~(1 << 6);
    f = f | (1 << 5);

    return 8;
}

std::uint8_t cpu::res_0_b()
{
    b = b & ~(1 << 0);

    return 8;
}

std::uint8_t cpu::res_0_c()
{
    c = c & ~(1 << 0);

    return 8;
}

std::uint8_t cpu::res_0_d()
{
    d = d & ~(1 << 0);

    return 8;
}

std::uint8_t cpu::res_0_e()
{
    e = e & ~(1 << 0);

    return 8;
}

std::uint8_t cpu::res_0_h()
{
    h = h & ~(1 << 0);

    return 8;
}

std::uint8_t cpu::res_0_l()
{
    l = l & ~(1 << 0);

    return 8;
}

std::uint8_t cpu::res_0_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 0);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_0_a()
{
    a = a & ~(1 << 0);

    return 8;
}

std::uint8_t cpu::res_1_b()
{
    b = b & ~(1 << 1);

    return 8;
}

std::uint8_t cpu::res_1_c()
{
    c = c & ~(1 << 1);

    return 8;
}

std::uint8_t cpu::res_1_d()
{
    d = d & ~(1 << 1);

    return 8;
}

std::uint8_t cpu::res_1_e()
{
    e = e & ~(1 << 1);

    return 8;
}

std::uint8_t cpu::res_1_h()
{
    h = h & ~(1 << 1);

    return 8;
}

std::uint8_t cpu::res_1_l()
{
    l = l & ~(1 << 1);

    return 8;
}

std::uint8_t cpu::res_1_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 1);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_1_a()
{
    a = a & ~(1 << 1);

    return 8;
}

std::uint8_t cpu::res_2_b()
{
    b = b & ~(1 << 2);

    return 8;
}

std::uint8_t cpu::res_2_c()
{
    c = c & ~(1 << 2);

    return 8;
}

std::uint8_t cpu::res_2_d()
{
    d = d & ~(1 << 2);

    return 8;
}

std::uint8_t cpu::res_2_e()
{
    e = e & ~(1 << 2);

    return 8;
}

std::uint8_t cpu::res_2_h()
{
    h = h & ~(1 << 2);

    return 8;
}

std::uint8_t cpu::res_2_l()
{
    l = l & ~(1 << 2);

    return 8;
}

std::uint8_t cpu::res_2_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 2);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_2_a()
{
    a = a & ~(1 << 2);

    return 8;
}

std::uint8_t cpu::res_3_b()
{
    b = b & ~(1 << 3);

    return 8;
}

std::uint8_t cpu::res_3_c()
{
    c = c & ~(1 << 3);

    return 8;
}

std::uint8_t cpu::res_3_d()
{
    d = d & ~(1 << 3);

    return 8;
}

std::uint8_t cpu::res_3_e()
{
    e = e & ~(1 << 3);

    return 8;
}

std::uint8_t cpu::res_3_h()
{
    h = h & ~(1 << 3);

    return 8;
}

std::uint8_t cpu::res_3_l()
{
    l = l & ~(1 << 3);

    return 8;
}

std::uint8_t cpu::res_3_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 3);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_3_a()
{
    a = a & ~(1 << 3);

    return 8;
}

std::uint8_t cpu::res_4_b()
{
    b = b & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::res_4_c()
{
    c = c & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::res_4_d()
{
    d = d & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::res_4_e()
{
    e = e & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::res_4_h()
{
    h = h & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::res_4_l()
{
    l = l & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::res_4_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 4);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_4_a()
{
    a = a & ~(1 << 4);

    return 8;
}

std::uint8_t cpu::res_5_b()
{
    b = b & ~(1 << 5);

    return 8;
}

std::uint8_t cpu::res_5_c()
{
    c = c & ~(1 << 5);

    return 8;
}

std::uint8_t cpu::res_5_d()
{
    d = d & ~(1 << 5);

    return 8;
}

std::uint8_t cpu::res_5_e()
{
    e = e & ~(1 << 5);

    return 8;
}

std::uint8_t cpu::res_5_h()
{
    h = h & ~(1 << 5);

    return 8;
}

std::uint8_t cpu::res_5_l()
{
    l = l & ~(1 << 5);

    return 8;
}

std::uint8_t cpu::res_5_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 5);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_5_a()
{
    a = a & ~(1 << 5);

    return 8;
}

std::uint8_t cpu::res_6_b()
{
    b = b & ~(1 << 6);

    return 8;
}

std::uint8_t cpu::res_6_c()
{
    c = c & ~(1 << 6);

    return 8;
}

std::uint8_t cpu::res_6_d()
{
    d = d & ~(1 << 6);

    return 8;
}

std::uint8_t cpu::res_6_e()
{
    e = e & ~(1 << 6);

    return 8;
}

std::uint8_t cpu::res_6_h()
{
    h = h & ~(1 << 6);

    return 8;
}

std::uint8_t cpu::res_6_l()
{
    l = l & ~(1 << 6);

    return 8;
}

std::uint8_t cpu::res_6_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 6);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_6_a()
{
    a = a & ~(1 << 6);

    return 8;
}

std::uint8_t cpu::res_7_b()
{
    b = b & ~(1 << 7);

    return 8;
}

std::uint8_t cpu::res_7_c()
{
    c = c & ~(1 << 7);

    return 8;
}

std::uint8_t cpu::res_7_d()
{
    d = d & ~(1 << 7);

    return 8;
}

std::uint8_t cpu::res_7_e()
{
    e = e & ~(1 << 7);

    return 8;
}

std::uint8_t cpu::res_7_h()
{
    h = h & ~(1 << 7);

    return 8;
}

std::uint8_t cpu::res_7_l()
{
    l = l & ~(1 << 7);

    return 8;
}

std::uint8_t cpu::res_7_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory & ~(1 << 7);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::res_7_a()
{
    a = a & ~(1 << 7);

    return 8;
}

std::uint8_t cpu::set_0_b()
{
    b = b | (1 << 0);

    return 8;
}

std::uint8_t cpu::set_0_c()
{
    c = c | (1 << 0);

    return 8;
}

std::uint8_t cpu::set_0_d()
{
    d = d | (1 << 0);

    return 8;
}

std::uint8_t cpu::set_0_e()
{
    e = e | (1 << 0);

    return 8;
}

std::uint8_t cpu::set_0_h()
{
    h = h | (1 << 0);

    return 8;
}

std::uint8_t cpu::set_0_l()
{
    l = l | (1 << 0);

    return 8;
}

std::uint8_t cpu::set_0_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 0);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_0_a()
{
    a = a | (1 << 0);

    return 8;
}

std::uint8_t cpu::set_1_b()
{
    b = b | (1 << 1);

    return 8;
}

std::uint8_t cpu::set_1_c()
{
    c = c | (1 << 1);

    return 8;
}

std::uint8_t cpu::set_1_d()
{
    d = d | (1 << 1);

    return 8;
}

std::uint8_t cpu::set_1_e()
{
    e = e | (1 << 1);

    return 8;
}

std::uint8_t cpu::set_1_h()
{
    h = h | (1 << 1);

    return 8;
}

std::uint8_t cpu::set_1_l()
{
    l = l | (1 << 1);

    return 8;
}

std::uint8_t cpu::set_1_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 1);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_1_a()
{
    a = a | (1 << 1);

    return 8;
}

std::uint8_t cpu::set_2_b()
{
    b = b | (1 << 2);

    return 8;
}

std::uint8_t cpu::set_2_c()
{
    c = c | (1 << 2);

    return 8;
}

std::uint8_t cpu::set_2_d()
{
    d = d | (1 << 2);

    return 8;
}

std::uint8_t cpu::set_2_e()
{
    e = e | (1 << 2);

    return 8;
}

std::uint8_t cpu::set_2_h()
{
    h = h | (1 << 2);

    return 8;
}

std::uint8_t cpu::set_2_l()
{
    l = l | (1 << 2);

    return 8;
}

std::uint8_t cpu::set_2_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 2);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_2_a()
{
    a = a | (1 << 2);

    return 8;
}

std::uint8_t cpu::set_3_b()
{
    b = b | (1 << 3);

    return 8;
}

std::uint8_t cpu::set_3_c()
{
    c = c | (1 << 3);

    return 8;
}

std::uint8_t cpu::set_3_d()
{
    d = d | (1 << 3);

    return 8;
}

std::uint8_t cpu::set_3_e()
{
    e = e | (1 << 3);

    return 8;
}

std::uint8_t cpu::set_3_h()
{
    h = h | (1 << 3);

    return 8;
}

std::uint8_t cpu::set_3_l()
{
    l = l | (1 << 3);

    return 8;
}

std::uint8_t cpu::set_3_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 3);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_3_a()
{
    a = a | (1 << 3);

    return 8;
}

std::uint8_t cpu::set_4_b()
{
    b = b | (1 << 4);

    return 8;
}

std::uint8_t cpu::set_4_c()
{
    c = c | (1 << 4);

    return 8;
}

std::uint8_t cpu::set_4_d()
{
    d = d | (1 << 4);

    return 8;
}

std::uint8_t cpu::set_4_e()
{
    e = e | (1 << 4);

    return 8;
}

std::uint8_t cpu::set_4_h()
{
    h = h | (1 << 4);

    return 8;
}

std::uint8_t cpu::set_4_l()
{
    l = l | (1 << 4);

    return 8;
}

std::uint8_t cpu::set_4_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 4);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_4_a()
{
    a = a | (1 << 4);

    return 8;
}

std::uint8_t cpu::set_5_b()
{
    b = b | (1 << 5);

    return 8;
}

std::uint8_t cpu::set_5_c()
{
    c = c | (1 << 5);

    return 8;
}

std::uint8_t cpu::set_5_d()
{
    d = d | (1 << 5);

    return 8;
}

std::uint8_t cpu::set_5_e()
{
    e = e | (1 << 5);

    return 8;
}

std::uint8_t cpu::set_5_h()
{
    h = h | (1 << 5);

    return 8;
}

std::uint8_t cpu::set_5_l()
{
    l = l | (1 << 5);

    return 8;
}

std::uint8_t cpu::set_5_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 5);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_5_a()
{
    a = a | (1 << 5);

    return 8;
}

std::uint8_t cpu::set_6_b()
{
    b = b | (1 << 6);

    return 8;
}

std::uint8_t cpu::set_6_c()
{
    c = c | (1 << 6);

    return 8;
}

std::uint8_t cpu::set_6_d()
{
    d = d | (1 << 6);

    return 8;
}

std::uint8_t cpu::set_6_e()
{
    e = e | (1 << 6);

    return 8;
}

std::uint8_t cpu::set_6_h()
{
    h = h | (1 << 6);

    return 8;
}

std::uint8_t cpu::set_6_l()
{
    l = l | (1 << 6);

    return 8;
}

std::uint8_t cpu::set_6_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 6);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_6_a()
{
    a = a | (1 << 6);

    return 8;
}

std::uint8_t cpu::set_7_b()
{
    b = b | (1 << 7);

    return 8;
}

std::uint8_t cpu::set_7_c()
{
    c = c | (1 << 7);

    return 8;
}

std::uint8_t cpu::set_7_d()
{
    d = d | (1 << 7);

    return 8;
}

std::uint8_t cpu::set_7_e()
{
    e = e | (1 << 7);

    return 8;
}

std::uint8_t cpu::set_7_h()
{
    h = h | (1 << 7);

    return 8;
}

std::uint8_t cpu::set_7_l()
{
    l = l | (1 << 7);

    return 8;
}

std::uint8_t cpu::set_7_addr_hl()
{
    std::uint16_t ahl = (h << 8) | l;
    std::uint8_t memory = read(ahl);
    memory = memory | (1 << 7);
    write(ahl, memory);

    return 16;
}

std::uint8_t cpu::set_7_a()
{
    a = a | (1 << 7);

    return 8;
}

std::uint8_t cpu::invalid()
{
    return 0;
}