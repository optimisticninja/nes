#include <gtest/gtest.h>

#include <iostream>

#include "../src/cpu.h"

void print_opcode(uint8_t opcode)
{
    cout << "\t\t0x" << hex << uppercase << setfill('0') << setw(2) << unsigned(opcode) << endl;
}

uint16_t get_zero_page_wrapped(uint16_t addr, uint8_t index_reg)
{
    uint16_t wrapped_addr = addr + index_reg;
    wrapped_addr -= wrapped_addr & 0xFF00;
    return wrapped_addr;
}

void setup_cpu(CPU& cpu, MappingMode mode)
{
    cpu.set_pc(0x0000);
    cpu.set_x(0x03);
    cpu.set_y(0x04);
    cpu.set_mem8(0x0001, 0x0F); // Set first operand

    switch (mode) {
        case ABSOLUTE:
            cpu.set_mem8(0x0002, 0xFF);                // Set second operand
            cpu.set_mem8(0xFF0F, 0x0F);                // Set target address value
            break;
        case ABSOLUTE_X:
            cpu.set_mem8(0x0002, 0xFF);                // Set second operand
            cpu.set_mem8(0xFF0F + cpu.get_x(), 0x0F);  // Set target address value
            break;
        case ABSOLUTE_Y:
            cpu.set_mem8(0x0002, 0xFF);                // Set second operand
            cpu.set_mem8(0xFF0F + cpu.get_y(), 0x0F);  // Set target address value
            break;
        case ACCUMULATOR:
            break;
        case IMMEDIATE:
            break;
        case IMPLICIT:
            break;
        case INDIRECT:
            break;
//         case INDIRECT_X:
//             cpu.set_mem8(0x0F0F, 0x0F);                // Indirect X
//             cpu.set_mem8(0x0F10, 0x00);                // Indirect X
//             break;
//         case INDIRECT_Y:
//             cpu.set_mem8(0x0013, 0x0F);                // Indirect Y
//             cpu.set_mem8(0x0014, 0x00);                // Indirect Y
//             break;
//         case RELATIVE:
//             break;
        case ZERO:
            cpu.set_mem8(0x000F, 0x0F);                // Set target address value
            break;
        case ZERO_X: {
            uint16_t address = get_zero_page_wrapped(0x000F, cpu.get_x());
            cpu.set_mem8(address, 0x0F);               // Set target address value
            break;
        }
        case ZERO_Y: {
            uint16_t address = get_zero_page_wrapped(0x000F, cpu.get_y());
            cpu.set_mem8(address, 0x0F);               // Set target address value
            break;
        }
        case NO_MAP:
        default:
            break;
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

/**************************************************************************************************************|| 
 * Using ORA instruction to test.
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

/***************************************************************************************************************/

/***************************************************************************************************************
 * Using STX instruction to test Zero Page Y.                                                                  */

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

TEST(CPU, BRK)
{
    CPU cpu = CPU();
    cpu.exec(0x00);
    ASSERT_EQ(cpu.get_p() & FLAG_INTERRUPT, FLAG_INTERRUPT);
    ASSERT_EQ(cpu.pull8(), cpu.get_p());
    ASSERT_EQ(cpu.pull16(), 1);
}

TEST(CPU, SEI)
{
    CPU cpu = CPU();
    cpu.exec(0x78);
    ASSERT_EQ(cpu.get_p() & FLAG_INTERRUPT, FLAG_INTERRUPT);
    ASSERT_EQ(cpu.get_pc(), INSTR_LEN[0x78]);
}

TEST(CPU, CLD)
{
    CPU cpu = CPU();
    cpu.exec(0xD8);
    ASSERT_EQ(cpu.get_p() & FLAG_DECIMAL, false);
    ASSERT_EQ(cpu.get_pc(), INSTR_LEN[0xD8]);
}
/*
TEST(CPU, LDA)
{
    CPU cpu = CPU();
    // Test negative flag
    cpu.set_mem8(0, 0xA9);
    cpu.set_mem8(1, 0xFF);
    cpu.exec(0xA9);
    ASSERT_EQ(cpu.get_a(), 0xFF);
    ASSERT_EQ(cpu.get_p() & FLAG_NEGATIVE, FLAG_NEGATIVE);
    ASSERT_EQ(cpu.get_pc(), INSTR_LEN[0xA9]);
    
    // Test zero flag
    cpu = CPU();
    cpu.set_a(0xFF);
    cpu.set_mem8(1, 0xFF);
    cpu.set_mem8(0, 0xA9);
    cpu.set_mem8(1, 0x00);
    cpu.exec(0xA9);
    ASSERT_EQ(cpu.get_a(), 0x00);
    ASSERT_EQ(cpu.get_p() & FLAG_ZERO, FLAG_ZERO);
}

TEST(CPU, LDX)
{
    CPU cpu = CPU();
    // Test negative flag
    cpu.set_x(0xEE);
    ASSERT_EQ(cpu.get_x(), 0xEE);
    cpu.set_mem8(0, 0xA2);
    cpu.set_mem8(1, 0xFF);
    cpu.exec(0xA2);
    ASSERT_EQ(cpu.get_x(), 0xFF);
    ASSERT_EQ(cpu.get_p() & FLAG_NEGATIVE, FLAG_NEGATIVE);
    ASSERT_EQ(cpu.get_pc(), INSTR_LEN[0xA2]);
}
*/

TEST(CPU, STA)
{
    // TODO: Uncomment when indirect x, y are tested */
    uint8_t opcodes[] = { 0x85, 0x95, 0x8D, 0x9D, 0x99 /*0x81, 0x91*/ };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xFF;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        cout << "\t\t0x" << hex << uppercase << setfill('0') << setw(2) << unsigned(opcodes[i]) << endl;
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]]);
        cpu.set_a(EXPECTED);
        cpu.exec(opcodes[i]);
        InstructionInfo info = cpu.get_curr_instr_info();
        ASSERT_EQ(cpu.get_mem8(info.addr), EXPECTED);
    }
}

TEST(CPU, ORA)
{
    // TODO: Uncomment when indirect x, y are tested */
    uint8_t opcodes[] = { 0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19 /*0x01, 0x11*/ };
    CPU cpu = CPU();
    const uint8_t EXPECTED = 0xF0 | 0x0F;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        print_opcode(opcodes[i]);
        setup_cpu(cpu, MAPPING_MODES[opcodes[i]]);
        cpu.set_a(0xF0);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}
