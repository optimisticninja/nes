#include <gtest/gtest.h>

#include <iostream>

#include "../src/cpu.h"

void print_opcode(uint8_t opcode)
{
    cout << "\t\t0x" << hex << uppercase << setfill('0') << setw(2) << unsigned(opcode) << endl;
}

uint16_t get_zero_page_wrapped(uint16_t addr, uint8_t index_reg)
{
    uint16_t wrapped_addr = (addr + index_reg) & 0x00FF;
    return wrapped_addr;
}

const uint16_t TARGET_ADDRESS = 0xFF0F;

void setup_cpu(CPU& cpu, MappingMode mode, bool load)
{
    cpu.set_pc(0x0000);
    cpu.set_x(0x03);
    cpu.set_y(0x04);
    cpu.set_mem8(0x0001, 0x0F); // Set first operand

    uint16_t address = 0x0000;
    
    switch (mode) {
        case ABSOLUTE:
            address = TARGET_ADDRESS;
            cpu.set_mem8(0x0002, 0xFF);                // Set second operand
            cpu.set_mem8(TARGET_ADDRESS, 0x0F);                // Set target address value
            break;
        case ABSOLUTE_X:
            address = TARGET_ADDRESS + cpu.get_x();
            cpu.set_mem8(0x0002, 0xFF);                // Set second operand
            cpu.set_mem8(address, 0x0F);  // Set target address value
            break;
        case ABSOLUTE_Y:
            address = TARGET_ADDRESS + cpu.get_y();
            cpu.set_mem8(0x0002, 0xFF);                // Set second operand
            cpu.set_mem8(address, 0x0F);  // Set target address value
            break;
        case ACCUMULATOR:
            break;
        case IMMEDIATE:
            address = 0x0001;
            break;
        case IMPLICIT:
            break;
        case INDIRECT:
            break;
        case INDIRECT_X:
            address = cpu.get_mem16(get_zero_page_wrapped(cpu.get_mem8(0x0001), cpu.get_x()));
            cpu.set_mem8(address, 0x0F);
            break;
        case INDIRECT_Y:
            address = cpu.get_mem16(cpu.get_mem8(0x0001)) + cpu.get_y();
            cpu.set_mem8(address, 0x0F);
            break;
        case RELATIVE:
            break;
        case ZERO:
            address = TARGET_ADDRESS & 0x00FF;
            cpu.set_mem8(address, 0x0F);                // Set target address value
            break;
        case ZERO_X: {
            address = get_zero_page_wrapped(TARGET_ADDRESS & 0x00FF, cpu.get_x());
            cpu.set_mem8(address, 0x0F);               // Set target address value
            break;
        }
        case ZERO_Y: {
            address = get_zero_page_wrapped(TARGET_ADDRESS & 0x00FF, cpu.get_y());
            cpu.set_mem8(address, 0x0F);               // Set target address value
            break;
        }
        case NO_MAP:
        default:
            break;
    }
    
    if (load) {
        cpu.set_mem8(address, 0xFF);
    }
}

TEST(CPU, PowerOnState)
{
    CPU cpu = CPU();
    ASSERT_EQ(cpu.get_p(), 0x34);
    ASSERT_EQ(cpu.get_a(), 0x00);
    ASSERT_EQ(cpu.get_x(), 0x00);
    ASSERT_EQ(cpu.get_y(), 0x00);
    ASSERT_EQ(cpu.get_s(), 0xFD);
    ASSERT_EQ(cpu.get_pc(), 0x0000);
}

/****************************************************************************************************************|| 
 * Using ORA instruction to test Zero, ZeroX, Absolute, AbsoluteX, AbxoluteY and Immediate, IndirectX, IndirectY
 *      - ORs the accumulator with the value in the address stored in the two bytes after the opcode
 *      - The expected value in accumulator should equate to 0xFF (0x0F | 0xF0)
 */

const uint16_t  ABSOLUTE_ADDRESS    = 0x0FEE;
const uint8_t   ORA_ABSOLUTE        = 0x0D;
const uint8_t   ORA_ABSOLUTE_X      = 0x1D;
const uint8_t   ORA_ABSOLUTE_Y      = 0x19;
const uint8_t   ORA_IMMEDIATE       = 0x09;
const uint8_t   ORA_ZERO            = 0x05;
const uint8_t   ORA_ZERO_X          = 0x15;
const uint8_t   ORA_INDIRECT_X      = 0x01;
const uint8_t   ORA_INDIRECT_Y      = 0x11;

TEST(AddressingModes, Zero)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_mem8(0x0000, ORA_ZERO); // Set opcode
    cpu.set_mem8(0x0001, 0xFF);     // Set next two bytes to address on zero page (0x00FF)
    cpu.set_mem8(0x00FF, 0x0F);     // Set zero page address's value to OR operand
    cpu.exec(ORA_ZERO);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

TEST(AddressingModes, ZeroX)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_x(0x03);
    cpu.set_mem8(0x0000, ORA_ZERO_X);         // Set opcode
    cpu.set_mem8(0x0001, 0xFF);               // Set next two bytes to address on zero page (0x00FF)
    uint16_t wrapped_addr = get_zero_page_wrapped(0x00FF, cpu.get_x());
    cpu.set_mem8(wrapped_addr, 0x0F);         // Set zero page address + register X's value to OR operand
    cpu.exec(ORA_ZERO_X);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

TEST(AddressingModes, Absolute)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_mem8(0x0000, ORA_ABSOLUTE);         // Set opcode
    cpu.set_mem16(0x0001, ABSOLUTE_ADDRESS);    // Set next two bytes to address
    cpu.set_mem8(ABSOLUTE_ADDRESS, 0x0F);       // Set value in absolute address
    cpu.exec(ORA_ABSOLUTE);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

TEST(AddressingModes, AbsoluteX)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_x(0x05);
    cpu.set_mem8(0x0000, ORA_ABSOLUTE_X);               // Set opcode
    cpu.set_mem16(0x0001, ABSOLUTE_ADDRESS);            // Set next two bytes to address
    cpu.set_mem8(ABSOLUTE_ADDRESS + cpu.get_x(), 0x0F); // Set value in absolute address + X register
    cpu.exec(ORA_ABSOLUTE_X);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

TEST(AddressingModes, AbsoluteY)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_x(0x05);
    cpu.set_mem8(0x0000, ORA_ABSOLUTE_Y);               // Set opcode
    cpu.set_mem16(0x0001, ABSOLUTE_ADDRESS);            // Set next two bytes to address
    cpu.set_mem8(ABSOLUTE_ADDRESS + cpu.get_y(), 0x0F); // Set value in absolute address + Y register
    cpu.exec(ORA_ABSOLUTE_Y);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

TEST(AddressingModes, Immediate)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_x(0x05);
    cpu.set_mem8(0x0000, ORA_IMMEDIATE);  // Set opcode
    cpu.set_mem8(0x0001, 0x0F);           // Set next byte as operand value for OR
    cpu.exec(ORA_IMMEDIATE);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

TEST(AddressingModes, IndirectX)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_x(0x00);
    cpu.set_mem8(0x0000, ORA_INDIRECT_X); // Set opcode
    cpu.set_mem8(0x0001, 0x02);           // Set next byte as zero page address
    uint16_t wrapped_addr = get_zero_page_wrapped(0x0002, cpu.get_x());
    uint16_t addr = cpu.get_mem16(wrapped_addr);
    cpu.set_mem16(addr, 0x8000);  // Set zero page wrapped value to target OR operand
    cpu.exec(ORA_INDIRECT_X);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

TEST(AddressingModes, IndirectY)
{
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_y(0x04);
    cpu.set_mem8(0x0000, ORA_INDIRECT_Y); // Set opcode
    cpu.set_mem8(0x0001, 0x02);           // Set next byte as zero page address
    cpu.set_mem16(0x0002, 0x8000);        // Set zero page area address
    cpu.set_mem8(0x8004, 0x0F);
    cpu.exec(ORA_INDIRECT_Y);
    ASSERT_EQ(cpu.get_a(), 0xFF);
}

/***************************************************************************************************************/

/***************************************************************************************************************
 * Using STX instruction to test ZeroY.                                                                  */

const uint8_t STX_ZERO_Y = 0x96;

TEST(AddressingModes, ZeroY)
{
    CPU cpu = CPU();
    cpu.set_x(0x03);
    cpu.set_y(0x05);            // Set Y as offset from zero page
    cpu.set_mem8(0x0001, 0xFF); // Set next byte as address on zero page
    cpu.exec(STX_ZERO_Y);
    uint16_t wrapped_addr = get_zero_page_wrapped(0x00FF, cpu.get_y());
    ASSERT_EQ(cpu.get_mem8(wrapped_addr), cpu.get_x());
}
/***************************************************************************************************************/

/* TODO: Relative for branch instructions */

TEST(CPU, Stack)
{
    CPU cpu = CPU();
    cpu.push8(0xFF);
    ASSERT_EQ(cpu.pull8(), 0xFF);
    cpu.push8(0xFE);
    ASSERT_EQ(cpu.pull8(), 0xFE);
    cpu.push16(0xFFFF);
    ASSERT_EQ(cpu.pull16(), 0xFFFF);
    cpu.push16(0xFFFD);
    ASSERT_EQ(cpu.pull16(), 0xFFFD);
}

TEST(Instructions, BRK)
{
    CPU cpu = CPU();
    cpu.exec(0x00);
    ASSERT_EQ(cpu.get_p() & FLAG_INTERRUPT, FLAG_INTERRUPT);
    ASSERT_EQ(cpu.pull8(), cpu.get_p());
    ASSERT_EQ(cpu.pull16(), 1);
}

TEST(Instructions, SEI)
{
    CPU cpu = CPU();
    cpu.exec(0x78);
    ASSERT_EQ(cpu.get_p() & FLAG_INTERRUPT, FLAG_INTERRUPT);
}

TEST(Instructions, SEC)
{
    CPU cpu = CPU();
    cpu.exec(0x38);
    ASSERT_EQ(cpu.get_p() & FLAG_CARRY, FLAG_CARRY);
}

TEST(Instructions, SED)
{
    CPU cpu = CPU();
    cpu.exec(0xF8);
    ASSERT_EQ(cpu.get_p() & FLAG_DECIMAL, FLAG_DECIMAL);
}

TEST(Instructions, CLC)
{
    CPU cpu = CPU();
    cpu.set_flag(FLAG_CARRY); // Set flag to make sure it is cleared on call
    cpu.exec(0x18);
    ASSERT_EQ(cpu.get_p() & FLAG_DECIMAL, false);
}

TEST(Instructions, CLD)
{
    CPU cpu = CPU();
    cpu.set_flag(FLAG_DECIMAL); // Set flag to make sure it is cleared on call
    cpu.exec(0xD8);
    ASSERT_EQ(cpu.get_p() & FLAG_DECIMAL, false);
}

TEST(Instructions, CLI)
{
    CPU cpu = CPU();
    cpu.set_flag(FLAG_INTERRUPT); // Set flag to make sure it is cleared on call
    cpu.exec(0x58);
    ASSERT_EQ(cpu.get_p() & FLAG_INTERRUPT, false);
}

TEST(Instructions, CLV)
{
    CPU cpu = CPU();
    cpu.set_flag(FLAG_OVERFLOW); // Set flag to make sure it is cleared on call
    cpu.exec(0xB8);
    ASSERT_EQ(cpu.get_p() & FLAG_OVERFLOW, false);
}

TEST(Instructions, CMP)
{
    uint8_t opcodes[] = { 0xC9, 0xC5, 0xD5, 0xCD, 0xDD, 0xD9, 0xC1, 0xD1 };
    CPU cpu = CPU();
    
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_a(0x0F);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_p() & FLAG_CARRY, FLAG_CARRY); // Test carry flag and zero
        ASSERT_EQ(cpu.get_p() & FLAG_ZERO, FLAG_ZERO); // Test carry flag and zero
    }
}

TEST(Instructions, CPX)
{
    uint8_t opcodes[] = { 0xE0, 0xE4, 0xEC };
    CPU cpu = CPU();

    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_x(0x0F);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_p() & FLAG_CARRY, FLAG_CARRY); // Test carry flag and zero
        ASSERT_EQ(cpu.get_p() & FLAG_ZERO, FLAG_ZERO); // Test carry flag and zero
    }
}

TEST(Instructions, CPY)
{
    uint8_t opcodes[] = { 0xC0, 0xC4, 0xCC };
    CPU cpu = CPU();
    
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_y(0x0F);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_p() & FLAG_CARRY, FLAG_CARRY); // Test carry flag and zero
        ASSERT_EQ(cpu.get_p() & FLAG_ZERO, FLAG_ZERO); // Test carry flag and zero
    }
}

TEST(Instructions, LDA)
{
    uint8_t opcodes[] = { 0xA9, 0xA5, 0xB5, 0xAD, 0xBD, 0xB9, 0xA1, 0xB1 };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], true);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}

TEST(Instructions, LDX)
{
    uint8_t opcodes[] = { 0xA2, 0xA6, 0xB6, 0xAE, 0xBE };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], true);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_x(), EXPECTED);
    }
}

TEST(Instructions, LDY)
{
    uint8_t opcodes[] = { 0xA0, 0xA4, 0xB4, 0xAC, 0xBC };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], true);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_y(), EXPECTED);
    }
}

TEST(Instructions, STA)
{
    uint8_t opcodes[] = { 0x85, 0x95, 0x8D, 0x9D, 0x99, 0x81, 0x91 };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_a(EXPECTED);
        cpu.exec(opcodes[i]);
        InstructionInfo info = cpu.get_curr_instr_info();
        ASSERT_EQ(cpu.get_mem8(info.addr), EXPECTED);
    }
}

TEST(Instructions, STX)
{
    uint8_t opcodes[] = { 0x86, 0x96, 0x8E };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_x(EXPECTED);
        cpu.exec(opcodes[i]);
        InstructionInfo info = cpu.get_curr_instr_info();
        ASSERT_EQ(cpu.get_mem8(info.addr), EXPECTED);
    }
}

TEST(Instructions, STY)
{
    uint8_t opcodes[] = { 0x84, 0x94, 0x8C };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_y(EXPECTED);
        cpu.exec(opcodes[i]);
        InstructionInfo info = cpu.get_curr_instr_info();
        ASSERT_EQ(cpu.get_mem8(info.addr), EXPECTED);
    }
}

TEST(Instructions, TAX)
{
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
    cpu.set_a(EXPECTED);
    cpu.exec(0xAA);
    ASSERT_EQ(cpu.get_x(), EXPECTED);
}

TEST(Instructions, TAY)
{
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
    cpu.set_a(EXPECTED);
    cpu.exec(0xA8);
    ASSERT_EQ(cpu.get_y(), EXPECTED);
}

TEST(Instructions, TSX)
{
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
    cpu.set_s(EXPECTED);
    cpu.exec(0xBA);
    ASSERT_EQ(cpu.get_x(), EXPECTED);
}

TEST(Instructions, TXA)
{
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
    cpu.set_x(EXPECTED);
    cpu.exec(0x8A);
    ASSERT_EQ(cpu.get_a(), EXPECTED);
}

TEST(Instructions, TXS)
{
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
    cpu.set_x(EXPECTED);
    cpu.exec(0x9A);
    ASSERT_EQ(cpu.get_s(), EXPECTED);
}

TEST(Instructions, TYA)
{
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
    cpu.set_y(EXPECTED);
    cpu.exec(0x98);
    ASSERT_EQ(cpu.get_a(), EXPECTED);
}

TEST(Instructions, ASL)
{
    uint8_t opcodes[] = { 0x0A, 0x06, 0x16, 0x0E, 0x1E };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x0F << 1;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_a(0x0F);
        cpu.exec(opcodes[i]);
        InstructionInfo info = cpu.get_curr_instr_info();
        
        if (info.mode == ACCUMULATOR)
            ASSERT_EQ(cpu.get_a(), EXPECTED);
        else
            ASSERT_EQ(cpu.get_mem8(info.addr), EXPECTED);
    }
}

TEST(Instructions, ORA)
{
    uint8_t opcodes[] = { 0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19, 0x01, 0x11 };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xF0 | 0x0F;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_a(0xF0);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}

TEST(Instructions, AND)
{
    uint8_t opcodes[] = { 0x29, 0x25, 0x35, 0x2D, 0x3D, 0x39, 0x21, 0x31 };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x0F & 0x0F;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_a(0x0F);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}

TEST(Instructions, EOR)
{
    uint8_t opcodes[] = { 0x49, 0x45, 0x55, 0x4D, 0x5D, 0x59, 0x41, 0x51 };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xF0 ^ 0x0F;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_a(0xF0);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}

TEST(Instructions, ADC)
{
    uint8_t opcodes[] = { 0x69, 0x65, 0x75, 0x6D, 0x7D, 0x79, 0x61, 0x71 };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xF0 + 0x0F;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.clear_flag(FLAG_CARRY); // Emulate calling CLC like a proper ADC call would
        cpu.set_a(0xF0);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}

TEST(Instructions, DEC)
{
    uint8_t opcodes[] = { 0xC6, 0xD6, 0xCE, 0xDE };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x0E;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.exec(opcodes[i]);
        InstructionInfo info = cpu.get_curr_instr_info();
        ASSERT_EQ(cpu.get_mem8(info.addr), EXPECTED);
    }
}

TEST(Instructions, DEX)
{
    uint8_t opcode = 0xCA;
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFE;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.set_x(0xFF);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.get_x(), EXPECTED);
}

TEST(Instructions, DEY)
{
    uint8_t opcode = 0x88;
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFE;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.set_y(0xFF);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.get_y(), EXPECTED);
}

TEST(Instructions, INC)
{
    uint8_t opcodes[] = { 0xE6, 0xF6, 0xEE, 0xFE };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x10;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.exec(opcodes[i]);
        InstructionInfo info = cpu.get_curr_instr_info();
        ASSERT_EQ(cpu.get_mem8(info.addr), EXPECTED);
    }
}

TEST(Instructions, INX)
{
    uint8_t opcode = 0xE8;
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x10;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.set_x(0x0F);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.get_x(), EXPECTED);
}

TEST(Instructions, INY)
{
    uint8_t opcode = 0xC8;
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x10;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.set_y(0x0F);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.get_y(), EXPECTED);
}

TEST(Instructions, SBC)
{
    uint8_t opcodes[] = { 0xE9, 0xE5, 0xF5, 0xED, 0xFD, 0xF9, 0xE1, 0xF1 };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF - 0x0F;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]], false);
        cpu.set_flag(FLAG_CARRY); // Emulate calling SEC like a proper SBC call would
        cpu.set_a(0xFF);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}

TEST(Instructions, PHA)
{
    uint8_t opcode = 0x48;
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x0F;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.set_a(EXPECTED);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.pull8(), EXPECTED);
}

TEST(Instructions, PHP)
{
    uint8_t opcode = 0x08;
    CPU cpu = CPU();
    const uint8_t EXPECTED = FLAG_CARRY | FLAG_DECIMAL | FLAG_INTERRUPT;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.set_p(EXPECTED);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.pull8(), EXPECTED);
}

TEST(Instructions, PLA)
{
    uint8_t opcode = 0x68;
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0x10;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.push8(EXPECTED);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.get_a(), EXPECTED);
}

TEST(Instructions, PLP)
{
    uint8_t opcode = 0x28;
    CPU cpu = CPU();
    const uint8_t EXPECTED = FLAG_CARRY | FLAG_NEGATIVE | FLAG_DECIMAL;
    setup_cpu(cpu, MAPPING_MODES[opcode], false);
    cpu.push8(EXPECTED);
    cpu.exec(opcode);
    ASSERT_EQ(cpu.get_p(), EXPECTED);
}
