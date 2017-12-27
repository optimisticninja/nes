#include <gtest/gtest.h>

#include <iostream>

#include "../src/cpu.h"

TEST(Memory, MemoryMap)
{
    CPU cpu = CPU();
    ASSERT_EQ(TOTAL_RAM_SIZE - 1, 0xFFFF);
    ASSERT_EQ(cpu.get_ram(), cpu.get_memptr(0x0000));
    ASSERT_EQ(cpu.get_mirror0(), cpu.get_memptr(0x0800));
    ASSERT_EQ(cpu.get_mirror1(), cpu.get_memptr(0x1000));
    ASSERT_EQ(cpu.get_mirror2(), cpu.get_memptr(0x1800));
    ASSERT_EQ(cpu.get_ppu_reg(), cpu.get_memptr(0x2000));
    ASSERT_EQ(cpu.get_ppu_reg_mirrors(), cpu.get_memptr(0x2008));
    ASSERT_EQ(cpu.get_apu_io_regs(), cpu.get_memptr(0x4000));
    ASSERT_EQ(cpu.get_apu_io_test_mode(), cpu.get_memptr(0x4018));
    ASSERT_EQ(cpu.get_cartridge_space(), cpu.get_memptr(0x4020));
}
