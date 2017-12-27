#include "cpu.h"

#ifdef DEBUG
    #include <iostream>
#endif

static const uint8_t FLAGS_IRQ_DISABLED = 0x34;

CPU::CPU()
{
    regs.p = FLAGS_IRQ_DISABLED;
    regs.a = regs.x = regs.y = 0;
    regs.s = 0xFD;
    this->regs.pc = 0x0000;
}

CPU::~CPU()
{
}

void CPU::exec(uint8_t opcode)
{
    uint16_t address = 0x0000;
    MappingMode mode = MAPPING_MODES[opcode];
    
    switch (mode) {
        case ABSOLUTE:
            address = this->get_mem16(this->regs.pc + 1);
            break;
        case ABSOLUTE_X:
            address = this->get_mem16(this->regs.pc + 1) + this->regs.x;
            break;
        case ABSOLUTE_Y:
            address = this->get_mem16(this->regs.pc + 1) + this->regs.y;
            break;
        case INDIRECT:
            address = this->get_mem16_bug(this->get_mem16(this->regs.pc + 1));
            break;
        case INDIRECT_X:
            address = this->get_mem16_bug(this->get_mem16(this->regs.pc + 1 + this->regs.x));
            break;
        case INDIRECT_Y:
            address = this->get_mem16_bug(this->get_mem16(this->regs.pc + 1)) + this->regs.y;
            break;
        case IMPLICIT:
            address = 0;
            break;
        case ACCUMULATOR:
            address = 0;
            break;
        case IMMEDIATE:
            address = this->regs.pc + 1;
            break;
        case ZERO:
            address = this->get_mem16(this->regs.pc + 1);
            break;
        case ZERO_X:
            address = this->get_mem16(this->regs.pc + 1 + this->regs.x);
            break;
        case ZERO_Y:
            address = this->get_mem16(this->regs.pc + 1 + this->regs.y);
            break;
        case RELATIVE: {
            uint16_t offset = this->get_mem8(this->regs.pc + 1);
            address = (offset < 0x80) 
                ? this->regs.pc + 2 + offset
                : this->regs.pc + 2 + offset - 0x100;
            break;
        }
        case NO_MAP: /* FALL THROUGH */
        default:
            cerr << "WARNING: Hit non-standard opcode or data!\n";
            break;
    };
    
    this->regs.pc += INSTR_LEN[opcode];
    
    this->curr_instr_info.addr = address;
    this->curr_instr_info.mode = mode;
    this->curr_instr_info.opcode = opcode;
    (this->*opcodes[opcode])(this->curr_instr_info);
}

void CPU::handle_flags(uint8_t flags, uint8_t val)
{
    if (flags & FLAG_CARRY) {
    }
    
    if (flags & FLAG_DECIMAL) {
    }
    
    if (flags & FLAG_INTERRUPT) {
    }
    
    if (flags & FLAG_NEGATIVE)
        (val & 0x80) != 0 
            ? this->regs.set_flag(FLAG_NEGATIVE) 
            : this->regs.clear_flag(FLAG_NEGATIVE);
    
    if (flags & FLAG_ZERO)
        val == 0 
            ? this->regs.set_flag(FLAG_ZERO) 
            : this->regs.clear_flag(FLAG_ZERO);
        
    
    if (flags & FLAG_OVERFLOW) {
    }
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

void CPU::brk(InstructionInfo& info)
{
//     this->regs.pc += 2;
    this->push16(this->regs.pc);
    this->push8(this->regs.p);
    this->sei(info);
    this->regs.set_flag(FLAG_INTERRUPT);
    this->regs.pc = this->get_mem16(0xFFFE);
}

void CPU::sta(InstructionInfo& info)
{
    this->set_mem8(info.addr, this->regs.a);
}

void CPU::sei(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.set_flag(FLAG_INTERRUPT);
}

void CPU::cld(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.clear_flag(FLAG_DECIMAL);
}

void CPU::lda(InstructionInfo& info)
{
    this->regs.a = this->get_mem8(info.addr);
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
}

void CPU::ldx(InstructionInfo& info)
{
    this->regs.x = this->get_mem8(info.addr);
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.x);
}

uint8_t CPU::get_a()
{
    return this->regs.a;
}

void CPU::set_a(uint8_t a)
{
    this->regs.a = a;
}

uint8_t CPU::get_x()
{
    return this->regs.x;
}

void CPU::set_x(uint8_t x)
{
    this->regs.x = x;
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

uint8_t* CPU::get_memptr(size_t i)
{
    return &this->mem[i];
}

uint8_t CPU::get_mem8(size_t i)
{
    return this->mem[i];
}

void CPU::set_mem8(size_t i, uint8_t val)
{
    this->mem[i] = val;
}

uint16_t CPU::get_mem16(size_t i)
{
    return *((uint16_t*) &this->mem[i]);
}

/* Bug for low byte wrap without incrementing high byte */
uint16_t CPU::get_mem16_bug(size_t i)
{
    uint16_t b = (i & 0xFF00) | (uint16_t) ((uint8_t) i + 1);
    uint8_t lo = this->get_mem8(i);
    uint8_t hi = this->get_mem8(b);
    return (uint16_t) hi << 8 | (uint16_t) lo;
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
