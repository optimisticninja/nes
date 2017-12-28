#include <gtest/gtest.h>

#include <iostream>

#include "../src/cpu.h"

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

TEST(CPU, STA)
{
    /* ABSOLUTE */
    CPU cpu = CPU();
    cpu.set_a(0xFF);
    ASSERT_EQ(cpu.get_a(), 0xFF);
    cpu.set_mem8(0, 0x8D);
    cpu.set_mem8(1, 0xFF);
    cpu.set_mem8(2, 0xFF);
    cpu.exec(0x8D);
    ASSERT_EQ(cpu.get_mem8(0xFFFF), cpu.get_a());
    ASSERT_EQ(cpu.get_pc(), INSTR_LEN[0x8D]);
}

/* TODO: Finish mapping modes and extract out to stub to be used in all tests */
TEST(CPU, ORA)
{
    uint8_t opcodes[8] = { 0x09, 0x05, 0x15, 0x0D, 0x1D, 0x19, 0x01, 0x11 };
    CPU cpu = CPU();
    cpu.set_a(0xF0);
    cpu.set_x(0x03);
    cpu.set_y(0x04);
    cpu.set_mem8(0x0001, 0x0F);                // Set first operand
    cpu.set_mem8(0x0002, 0xFF);                // Set second operand
    cpu.set_mem8(0x000F, 0x0F);                // Zero paged
    cpu.set_mem8(0x0001 + cpu.get_x(), 0x0F);  // Zero paged X
    cpu.set_mem8(0x0001 + cpu.get_y(), 0x0F);  // Zero paged Y
    cpu.set_mem8(0xFF0F, 0x0F);                // Absolute
    cpu.set_mem8(0xFF0F + cpu.get_x(), 0x0F);  // Absolute X
    cpu.set_mem8(0xFF0F + cpu.get_y(), 0x0F);  // Absolute Y
    cpu.set_mem8(0x0F0F, 0x0F);                // Indirect X
    cpu.set_mem8(0x0F10, 0x00);                // Indirect X
    cpu.set_mem8(0x0013, 0x0F);                // Indirect Y
    cpu.set_mem8(0x0014, 0x00);                // Indirect Y
    
    const uint8_t EXPECTED = 0xF0 | 0x0F;
        
    for (unsigned i = 0; i < sizeof(opcodes) / sizeof(uint8_t); i++) {
        cout << "\t\t0x" << hex << uppercase << setfill('0') << setw(2) << unsigned(opcodes[i]) << endl;
        cpu.set_pc(0x0000);
        cpu.set_a(0xF0);
        cpu.exec(opcodes[i]);
        ASSERT_EQ(cpu.get_a(), EXPECTED);
    }
}
