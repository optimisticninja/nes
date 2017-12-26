#include "cpu.h"

#include <iostream>

static const uint8_t FLAGS_IRQ_DISABLED = 0x34;

CPU::CPU() :
    regs(new Regs)
{
    regs->p = FLAGS_IRQ_DISABLED;
    regs->a = regs->x = regs->y = 0;
    regs->s = 0xFD;
}

CPU::~CPU()
{
    delete this->regs;
}

void CPU::exec(uint8_t opcode)
{
    uint16_t address;
    MappingMode mode = MAPPING_MODES[opcode];
    
    switch (mode) {
        case ZERO_X:
            break;
        case ZERO_Y:
            break;
        case ABSOLUTE_X:
            break;
        case ABSOLUTE_Y:
            break;
        case INDIRECT_X:
            break;
        case INDIRECT_Y:
            break;
        case IMPLICIT:
            break;
        case ACCUMULATOR:
            break;
        case IMMEDIATE:
            address = this->regs->pc + 1;
            break;
        case ZERO:
            break;
        case ABSOLUTE:
            break;
        case RELATIVE:
            break;
        case INDIRECT:
            break;
        case NO_MAP:
            break;
        default:
            break;
    };
    
    this->curr_instr_info.addr = address;
    this->curr_instr_info.mode = mode;
    this->curr_instr_info.opcode = opcode;
    
    (this->*opcodes[opcode])(this->curr_instr_info);
}

void CPU::push8(uint8_t val)
{
    this->stack[this->regs->s] = val;
    this->regs->s--;
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
    this->regs->s++;
    return this->stack[this->regs->s];
}

uint16_t CPU::pull16()
{
    uint16_t lo = this->pull8();
    uint16_t hi = this->pull8();
    return hi << 8 | lo;
}

void CPU::brk(__attribute__((unused)) InstructionInfo& info)
{
    this->regs->pc += BRK_00_LEN;
    this->push16(this->regs->pc);
    this->push8(this->regs->p);
    this->regs->set_flag(FLAG_INTERRUPT);
    this->regs->pc = (uint16_t) ((*this->get_memb(0xFFFE) << 8) | *this->get_memb(0xFFFF));
}

void CPU::sei(__attribute__((unused)) InstructionInfo& info)
{
    this->regs->pc += 1;
    this->regs->set_flag(FLAG_INTERRUPT);
}

void CPU::cld(__attribute__((unused)) InstructionInfo& info)
{
    this->regs->pc += 1;
    this->regs->clear_flag(FLAG_DECIMAL);
}

void CPU::lda(InstructionInfo& info)
{
    this->regs->pc += 1;
    
    uint8_t immediate;
    
    switch (info.mode) {
        case IMMEDIATE:
            immediate = *this->get_memb(info.addr);
            
            if (immediate > 0x7F)
                this->regs->clear_flag(FLAG_NEGATIVE);
            else if (immediate == 0x00)
                this->regs->set_flag(FLAG_ZERO);
            
            this->regs->a = immediate;
            break;
        default:
            break;
    }
}

uint8_t CPU::get_a()
{
    return this->regs->a;
}

void CPU::set_a(uint8_t a)
{
    this->regs->a = a;
}

uint8_t CPU::get_x()
{
    return this->regs->x;
}

uint8_t CPU::get_y()
{
    return this->regs->y;
}

uint16_t CPU::get_pc()
{
    return this->regs->pc;
}

uint8_t CPU::get_s()
{
    return this->regs->s;
}

uint8_t CPU::get_p()
{
    return this->regs->p;
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
