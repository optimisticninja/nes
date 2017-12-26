#include "cpu.h"

static const uint8_t FLAGS_IRQ_DISABLED = 0x34;

CPU::CPU()
{
    regs.p = FLAGS_IRQ_DISABLED;
    regs.a = regs.x = regs.y = 0;
    regs.s = 0xFD;
}

CPU::~CPU()
{
}

void CPU::exec(uint8_t opcode)
{
    (this->*opcodes[opcode])();
}

void CPU::push8(uint8_t val)
{
    this->stack[this->regs.s] = val;
    this->regs.s--;
}

void CPU::push16(uint16_t val)
{
    uint8_t hi = val >> 8;
    uint8_t lo = val & 0xFF;
    this->push8(hi);
    this->push8(lo);
}

uint8_t CPU::pull8()
{
    this->regs.s++;
    return this->stack[this->regs.s];
}

uint16_t CPU::pull16()
{
    uint16_t lo = this->pull8();
    uint16_t hi = this->pull8();
    return hi << 8 | lo;
}

void CPU::brk_00()
{
    this->regs.pc += BRK_00_LEN;
    this->push16(this->regs.pc);
    this->push8(this->regs.p);
    this->regs.p |= FLAG_INTERRUPT;
    this->regs.pc = (uint16_t) ((*this->get_memb(0xFFFE) << 8) | *this->get_memb(0xFFFF));
}

void CPU::ora_01()
{
    
}

uint8_t CPU::get_a()
{
    return this->regs.a;
}

uint8_t CPU::get_x()
{
    return this->regs.x;
}

uint8_t CPU::get_y()
{
    return this->regs.y;
}

uint16_t CPU::get_pc()
{
    return this->regs.pc;
}

uint8_t CPU::get_s()
{
    return this->regs.s;
}

uint8_t CPU::get_p()
{
    return this->regs.p;
}

uint8_t * CPU::get_memb(size_t i)
{
    return &this->mem[i];
}

void CPU::set_memb(size_t i, uint8_t val)
{
    this->mem[i] = val;
}

uint8_t* CPU::get_apu_io_regs()
{
    return this->apu_io_regs;
}

uint8_t* CPU::get_apu_io_test_mode()
{
    return this->apu_io_test_mode;
}

uint8_t* CPU::get_cartridge_space()
{
    return this->cartridge_space;
}

uint8_t* CPU::get_mirror0()
{
    return this->mirror0;
}

uint8_t* CPU::get_mirror1()
{
    return this->mirror1;
}

uint8_t* CPU::get_mirror2()
{
    return this->mirror2;
}

uint8_t* CPU::get_ppu_reg()
{
    return this->ppu_reg;
}

uint8_t* CPU::get_ppu_reg_mirrors()
{
    return this->ppu_reg_mirrors;
}

uint8_t* CPU::get_ram()
{
    return this->ram;
}
