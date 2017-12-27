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
    ASSERT_EQ(cpu.get_pc(), 1);
}

TEST(CPU, CLD)
{
    CPU cpu = CPU();
    cpu.exec(0xD8);
    ASSERT_EQ(cpu.get_p() & FLAG_DECIMAL, false);
    ASSERT_EQ(cpu.get_pc(), 1);
}

TEST(CPU, LDA_Immediate)
{
    CPU cpu = CPU();
    // Test negative flag
    cpu.set_mem8(0, 0xA9);
    cpu.set_mem8(1, 0xFF);
    cpu.exec(0xA9);
    ASSERT_EQ(cpu.get_a(), 0xFF);
    ASSERT_EQ(cpu.get_p() & FLAG_NEGATIVE, FLAG_NEGATIVE);
    
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

TEST(CPU, LDX_IndirectX)
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
}

TEST(CPU, STA_Absolute)
{
    CPU cpu = CPU();
    cpu.set_a(0xFF);
    ASSERT_EQ(cpu.get_a(), 0xFF);
    cpu.set_mem8(0, 0x8D);
    cpu.set_mem8(1, 0xFF);
    cpu.set_mem8(2, 0xFF);
    cpu.exec(0x8D);
    ASSERT_EQ(cpu.get_mem8(0xFFFF), cpu.get_a());
    ASSERT_EQ(cpu.get_pc(), 3);
}
