#include "cpu.h"

#ifdef DEBUG
    #include <iostream>
#endif

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
            //address = this->get_mem16_bug(this->get_mem16(this->regs.pc + 1));
            break;
        case INDIRECT_X:
            address = (this->mem[this->regs.pc + 1] + this->regs.x) & 0x00FF; // Ignore carry and wrap on zero page 
            break;
        case INDIRECT_Y:
            address = this->get_mem16(this->mem[this->regs.pc + 1]) + this->regs.y;
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
            address = this->mem[this->regs.pc + 1];
            break;
        case ZERO_X:
            address = (this->mem[this->regs.pc + 1] + this->regs.x)  & 0x00FF; // Ignore carry and wrap on zero page
            break;
        case ZERO_Y:
            address = (this->mem[this->regs.pc + 1] + this->regs.y)  & 0x00FF; // Ignore carry and wrap on zero page;
            break;
        case RELATIVE: {
            uint16_t offset = this->mem[this->regs.pc + 1];
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
    this->push16(this->regs.pc);
    this->push8(this->regs.p);
    this->sei(info);
    this->regs.set_flag(FLAG_INTERRUPT);
    this->regs.pc = this->get_mem16(0xFFFE);
}

void CPU::nop(__attribute__((unused)) InstructionInfo& info)
{
    /* NOPping... */
}

void CPU::sta(InstructionInfo& info)
{
    this->mem[info.addr] = this->regs.a;
}

void CPU::stx(InstructionInfo& info)
{
    this->mem[info.addr] = this->regs.x;
}

void CPU::sty(InstructionInfo& info)
{
    this->mem[info.addr] = this->regs.y;
}

void CPU::tax(__attribute__((unused)) InstructionInfo& info)
{
    handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
    this->regs.x = this->regs.a;
}

void CPU::tay(__attribute__((unused)) InstructionInfo& info)
{
    handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
    this->regs.y = this->regs.a;
}

void CPU::tsx(__attribute__((unused)) InstructionInfo& info)
{
    handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.s);
    this->regs.x = this->regs.s;
}

void CPU::txa(__attribute__((unused)) InstructionInfo& info)
{
    handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.x);
    this->regs.a = this->regs.x;
}

void CPU::txs(__attribute__((unused)) InstructionInfo& info)
{
    handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.x);
    this->regs.s = this->regs.x;
}

void CPU::tya(__attribute__((unused)) InstructionInfo& info)
{
    handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.y);
    this->regs.a = this->regs.y;
}


void CPU::sed(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.set_flag(FLAG_DECIMAL);
}

void CPU::clc(__attribute((unused)) InstructionInfo& info)
{
    this->regs.clear_flag(FLAG_CARRY);
}

void CPU::cld(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.clear_flag(FLAG_DECIMAL);
}

void CPU::cli(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.clear_flag(FLAG_INTERRUPT);
}

void CPU::clv(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.clear_flag(FLAG_OVERFLOW);
}

void CPU::sei(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.set_flag(FLAG_INTERRUPT);
}

void CPU::sec(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.set_flag(FLAG_CARRY);
}

void CPU::cmp(InstructionInfo& info)
{
    uint8_t a = this->regs.a;
    uint8_t m = this->mem[info.addr];
    
    if (a >= m)
        this->sec(info);
    
    if (a == m)
        this->set_flag(FLAG_ZERO);
    
    this->handle_flags(FLAG_NEGATIVE, a - m);
}

void CPU::cpx(InstructionInfo& info)
{
    uint8_t x = this->regs.x;
    uint8_t m = this->mem[info.addr];
    
    if (x >= m)
        this->sec(info);
    
    if (x == m)
        this->set_flag(FLAG_ZERO);
    
    this->handle_flags(FLAG_NEGATIVE, x - m);
}

void CPU::cpy(InstructionInfo& info)
{
    uint8_t y = this->regs.y;
    uint8_t m = this->mem[info.addr];
    
    if (y >= m)
        this->sec(info);
    
    if (y == m)
        this->set_flag(FLAG_ZERO);
    
    this->handle_flags(FLAG_NEGATIVE, y - m);
}

void CPU::lda(InstructionInfo& info)
{
    this->regs.a = this->mem[info.addr];
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
}

void CPU::ldx(InstructionInfo& info)
{
    this->regs.x = this->mem[info.addr];
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.x);
}

void CPU::ldy(InstructionInfo& info)
{
    this->regs.y = this->mem[info.addr];
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.y);
}

void CPU::asl(InstructionInfo& info)
{
    int a = (info.mode == ACCUMULATOR) ? this->regs.a : this->mem[info.addr];
    cout << "a: " << a << endl;
    if (a & (1 << 7))
        this->set_flag(FLAG_CARRY);
    
    a <<= 1;
        cout << "a: " << a << endl;

    if (info.mode == ACCUMULATOR) {
        this->regs.a <<= 1;
        this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
    } else {
        this->mem[info.addr] = a;
        this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->mem[info.addr]);
    }
}

void CPU::ora(InstructionInfo& info)
{
    this->regs.a |= this->mem[info.addr];
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
}

void CPU::_and(InstructionInfo& info)
{
    this->regs.a &= this->mem[info.addr];
    this->handle_flags(FLAG_ZERO  | FLAG_NEGATIVE, this->regs.a);
}

void CPU::eor(InstructionInfo& info)
{
    this->regs.a ^= this->mem[info.addr];
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
}

void CPU::adc(InstructionInfo& info)
{
    uint8_t a = this->regs.a;
    uint8_t b = this->mem[info.addr];
    uint8_t c = this->get_p() & FLAG_CARRY;
    
    this->regs.a = a + b + c;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
    
    (int) (a + b + c) > 0xFF
        ? this->regs.set_flag(FLAG_CARRY) 
        : this->regs.clear_flag(FLAG_CARRY);
    
    ((a ^ b) & 0x80) == 0 && ((a ^ this->regs.a) & 0x80) != 0 
        ? this->regs.set_flag(FLAG_OVERFLOW) 
        : this->regs.clear_flag(FLAG_OVERFLOW);
}

void CPU::dec(InstructionInfo& info)
{
    this->mem[info.addr] -= 1;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->mem[info.addr]);
}

void CPU::dex(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.x -= 1;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.x);
}

void CPU::dey(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.y -= 1;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.y);
}

void CPU::inc(InstructionInfo& info)
{
    this->mem[info.addr] += 1;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->mem[info.addr]);
}

void CPU::inx(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.x += 1;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.x);
}

void CPU::iny(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.y += 1;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.y);
}

void CPU::sbc(InstructionInfo& info)
{
    uint8_t a = this->regs.a;
    uint8_t b = this->mem[info.addr];
    uint8_t c = 1 - (this->get_p() & FLAG_CARRY);
    
    this->regs.a = a - b - c;
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
    
    (int) (a - b - c) >= 0x00
        ? this->regs.set_flag(FLAG_CARRY) 
        : this->regs.clear_flag(FLAG_CARRY);
    
    ((a ^ b) & 0x80) != 0 && ((a ^ this->regs.a) & 0x80) != 0 
        ? this->regs.set_flag(FLAG_OVERFLOW) 
        : this->regs.clear_flag(FLAG_OVERFLOW);
}

void CPU::pha(__attribute__((unused)) InstructionInfo& info)
{
    this->push8(this->regs.a);
}

void CPU::php(__attribute__((unused)) InstructionInfo& info)
{
    this->push8(this->regs.p);
}

void CPU::pla(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.a = this->pull8();
    this->handle_flags(FLAG_ZERO | FLAG_NEGATIVE, this->regs.a);
}

void CPU::plp(__attribute__((unused)) InstructionInfo& info)
{
    this->regs.p = this->pull8();
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

void CPU::set_y(uint8_t y)
{
    this->regs.y = y;
}

uint16_t CPU::get_pc()
{
    return this->regs.pc;
}

void CPU::set_pc(uint16_t pc)
{
    this->regs.pc = pc;
}

uint8_t CPU::get_s()
{
    return this->regs.s;
}

void CPU::set_s(uint8_t s)
{
    this->regs.s = s;
}

uint8_t CPU::get_p()
{
    return this->regs.p;
}

void CPU::set_p(uint8_t p)
{
    this->regs.p = p;
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
    uint16_t lo = this->mem[i];
    uint16_t hi = this->mem[i + 1];
    return hi << 8 | lo;
}

void CPU::set_mem16(size_t i, uint16_t val)
{
    *((uint16_t*) &this->mem[i]) = val;
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

InstructionInfo CPU::get_curr_instr_info()
{
    return this->curr_instr_info;
}

void CPU::clear_flag(Flag flag)
{
    this->regs.clear_flag(flag);
}

void CPU::set_flag(Flag flag)
{
    this->regs.set_flag(flag);
}
