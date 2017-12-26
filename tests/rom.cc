#include <gtest/gtest.h>

#include "../src/rom.h"

TEST(ROM, HeaderTest)
{
    ROM rom = ROM("rom/Super Mario Bros (E).nes");
    ASSERT_EQ(rom.get_prg_rom_pages(), 2);
    ASSERT_EQ(rom.get_char_rom_pages(), 1);
    ASSERT_EQ(rom.get_flags6(), 0b00000001);
    ASSERT_EQ(rom.get_flags7(), 0b00000000);
}
