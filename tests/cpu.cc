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

TEST(CPU, MemoryMap)
{
    CPU cpu = CPU();
    ASSERT_EQ(TOTAL_RAM_SIZE - 1, 0xFFFF);
    ASSERT_EQ(cpu.get_ram(), cpu.get_memb(0x0000));
    ASSERT_EQ(cpu.get_mirror0(), cpu.get_memb(0x0800));
    ASSERT_EQ(cpu.get_mirror1(), cpu.get_memb(0x1000));
    ASSERT_EQ(cpu.get_mirror2(), cpu.get_memb(0x1800));
    ASSERT_EQ(cpu.get_ppu_reg(), cpu.get_memb(0x2000));
    ASSERT_EQ(cpu.get_ppu_reg_mirrors(), cpu.get_memb(0x2008));
    ASSERT_EQ(cpu.get_apu_io_regs(), cpu.get_memb(0x4000));
    ASSERT_EQ(cpu.get_apu_io_test_mode(), cpu.get_memb(0x4018));
    ASSERT_EQ(cpu.get_cartridge_space(), cpu.get_memb(0x4020));
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
    ASSERT_EQ(cpu.pull16(), BRK_00_LEN);
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
    cpu.set_memb(0, 0xA9);
    cpu.set_memb(1, 0xFF);
    cpu.exec(0xA9);
    ASSERT_EQ(cpu.get_a(), 0xFF);
    ASSERT_EQ(cpu.get_p() & FLAG_NEGATIVE, false);
    
    // Test zero flag
    cpu = CPU();
    cpu.set_a(0xFF);
    cpu.set_memb(0, 0xA9);
    cpu.set_memb(1, 0x00);
    cpu.exec(0xA9);
    ASSERT_EQ(cpu.get_a(), 0x00);
    ASSERT_EQ(cpu.get_p() & FLAG_ZERO, FLAG_ZERO);
}
