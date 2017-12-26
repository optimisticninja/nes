#pragma once

#include <cstdint>

using namespace std;

const uint16_t STACK_ADDR = 0x100;

enum Flags : uint8_t {
    FLAG_CARRY       = 1 << 0,
    FLAG_ZERO        = 1 << 1,
    FLAG_INTERRUPT   = 1 << 2,
    FLAG_DECIMAL     = 1 << 3,
    FLAG_OVERFLOW    = 1 << 6,
    FLAG_NEGATIVE    = 1 << 7
};

enum NumMirrors : size_t {
    NUM_RAM_MIRRORS = 3,
    NUM_PPU_REGS_MIRRORS = 1023
};

enum MemSize : size_t {
    RAM_SIZE = 0x800,
    RAM_MIRROR_SIZE = 0x800,
    PPU_REGS_SIZE = 0x8,
    PPU_MIRROR_SIZE = 0x1FF8,
    APU_IO_REGS_SIZE = 0x18,
    APU_IO_TEST_MODE_SIZE = 0x8,
    CARTRIDGE_SPACE_SIZE = 0xBFE0,
    TOTAL_RAM_SIZE =
        RAM_SIZE +
        RAM_MIRROR_SIZE * NUM_RAM_MIRRORS +
        PPU_REGS_SIZE +
        PPU_REGS_SIZE * NUM_PPU_REGS_MIRRORS +
        APU_IO_REGS_SIZE +
        APU_IO_TEST_MODE_SIZE +
        CARTRIDGE_SPACE_SIZE
};

enum MappingMode {
    // Indexed
    ZERO_X,
    ZERO_Y,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    INDIRECT_X,
    INDIRECT_Y,
    // Other
    IMPLICIT,
    ACCUMULATOR,
    IMMEDIATE,
    ZERO,
    ABSOLUTE,
    RELATIVE,
    INDIRECT,
    NO_MAP
};

struct Regs {
    uint8_t     a;      // Accumulator
    uint8_t     x, y;   // Indexes
    uint16_t    pc;     // Program counter
    uint8_t     s;      // Stack pointer
    uint8_t     p;      // Status register
};

const size_t NUM_OPCODES = 256;

const MappingMode MAPPING_MODES[NUM_OPCODES] = {
  /*     0x00        0x01       0x02    0x03    0x04    0x05    0x06    0x07      0x08        0x09         0x0A    0x0B        0x0C        0x0D        0x0E    0x0F  */
     IMPLICIT, INDIRECT_X,    NO_MAP, NO_MAP, NO_MAP,   ZERO,   ZERO, NO_MAP, IMPLICIT,  IMMEDIATE, ACCUMULATOR, NO_MAP,     NO_MAP,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0x0F
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, NO_MAP, ZERO_X, ZERO_X, NO_MAP, IMPLICIT, ABSOLUTE_Y,      NO_MAP, NO_MAP,     NO_MAP, ABSOLUTE_X, ABSOLUTE_X, NO_MAP, // 0x1F
     IMPLICIT, INDIRECT_X,    NO_MAP, NO_MAP,   ZERO,   ZERO,   ZERO, NO_MAP, IMPLICIT,  IMMEDIATE, ACCUMULATOR, NO_MAP,   ABSOLUTE,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0x2F
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, NO_MAP, ZERO_X, ZERO_X, NO_MAP, IMPLICIT, ABSOLUTE_Y,      NO_MAP, NO_MAP,     NO_MAP, ABSOLUTE_X, ABSOLUTE_X, NO_MAP, // 0x3F
     IMPLICIT, INDIRECT_X,    NO_MAP, NO_MAP, NO_MAP,   ZERO,   ZERO, NO_MAP, IMPLICIT,  IMMEDIATE, ACCUMULATOR, NO_MAP,   ABSOLUTE,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0x4F
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, NO_MAP, ZERO_X, ZERO_X, NO_MAP, IMPLICIT, ABSOLUTE_Y,      NO_MAP, NO_MAP,     NO_MAP, ABSOLUTE_X, ABSOLUTE_X, NO_MAP, // 0x5F
     IMPLICIT, INDIRECT_X,    NO_MAP, NO_MAP, NO_MAP,   ZERO,   ZERO, NO_MAP, IMPLICIT,  IMMEDIATE, ACCUMULATOR, NO_MAP,   INDIRECT,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0x6F
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, NO_MAP, ZERO_X, ZERO_X, NO_MAP, IMPLICIT, ABSOLUTE_Y,      NO_MAP, NO_MAP,     NO_MAP, ABSOLUTE_X, ABSOLUTE_X, NO_MAP, // 0x7F
       NO_MAP, INDIRECT_X,    NO_MAP, NO_MAP,   ZERO,   ZERO,   ZERO, NO_MAP, IMPLICIT,     NO_MAP,    IMPLICIT, NO_MAP,   ABSOLUTE,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0x8F
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, ZERO_X, ZERO_X, ZERO_Y, NO_MAP, IMPLICIT, ABSOLUTE_Y,    IMPLICIT, NO_MAP,     NO_MAP, ABSOLUTE_X,     NO_MAP, NO_MAP, // 0x9F
    IMMEDIATE, INDIRECT_X, IMMEDIATE, NO_MAP,   ZERO,   ZERO,   ZERO, NO_MAP, IMPLICIT,  IMMEDIATE,    IMPLICIT, NO_MAP,   ABSOLUTE,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0xAF
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, ZERO_X, ZERO_X, ZERO_Y, NO_MAP, IMPLICIT, ABSOLUTE_Y,    IMPLICIT, NO_MAP, ABSOLUTE_X, ABSOLUTE_X, ABSOLUTE_Y, NO_MAP, // 0xBF
    IMMEDIATE, INDIRECT_X,    NO_MAP, NO_MAP,   ZERO,   ZERO,   ZERO, NO_MAP, IMPLICIT,  IMMEDIATE,    IMPLICIT, NO_MAP,   ABSOLUTE,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0xCF
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, NO_MAP, ZERO_X, ZERO_X, NO_MAP, IMPLICIT, ABSOLUTE_Y,      NO_MAP, NO_MAP,     NO_MAP, ABSOLUTE_X, ABSOLUTE_X, NO_MAP, // 0xDF
    IMMEDIATE, INDIRECT_X,    NO_MAP, NO_MAP,   ZERO,   ZERO,   ZERO, NO_MAP, IMPLICIT,  IMMEDIATE,    IMPLICIT, NO_MAP,   ABSOLUTE,   ABSOLUTE,   ABSOLUTE, NO_MAP, // 0xEF
     IMPLICIT, INDIRECT_Y,    NO_MAP, NO_MAP, NO_MAP, ZERO_X, ZERO_X, NO_MAP, IMPLICIT, ABSOLUTE_Y,      NO_MAP, NO_MAP,     NO_MAP, ABSOLUTE_X, ABSOLUTE_X, NO_MAP  // 0xFF
};

enum InstructionLength : uint8_t {
    BRK_00_LEN = 2
};

class CPU
{
private:
    Regs        regs;
    uint8_t     mem[TOTAL_RAM_SIZE - 1];
    uint8_t*    stack = &this->mem[STACK_ADDR];
    uint8_t*    ram = &mem[0];
    uint8_t*    mirror0 = ram + RAM_SIZE;
    uint8_t*    mirror1 = mirror0 + RAM_MIRROR_SIZE;
    uint8_t*    mirror2 = mirror1 + RAM_MIRROR_SIZE;
    uint8_t*    ppu_reg = mirror2 + RAM_MIRROR_SIZE;
    uint8_t*    ppu_reg_mirrors = ppu_reg + PPU_REGS_SIZE;
    uint8_t*    apu_io_regs = ppu_reg_mirrors + PPU_REGS_SIZE * NUM_PPU_REGS_MIRRORS;
    uint8_t*    apu_io_test_mode = apu_io_regs + APU_IO_REGS_SIZE;
    uint8_t*    cartridge_space = apu_io_test_mode + APU_IO_TEST_MODE_SIZE;
    
    void        brk_00();
    void        ora_01();
    
public:
    CPU();
    ~CPU();
    
    void (CPU::*opcodes[NUM_OPCODES])() = {
        &CPU::brk_00, &CPU::ora_01
    };
    
    void        exec(uint8_t opcode);
    
    /* STACK */
    void        push8(uint8_t val);
    void        push16(uint16_t val);
    
    uint8_t     pull8();
    uint16_t    pull16();
    
    uint8_t     get_a();
    uint8_t     get_x();
    uint8_t     get_y();
    uint16_t    get_pc();
    uint8_t     get_s();
    uint8_t     get_p();
    
    uint8_t*    get_memb(size_t i);
    void        set_memb(size_t i, uint8_t val);
    
    uint8_t*    get_ram();
    uint8_t*    get_mirror0();
    uint8_t*    get_mirror1();
    uint8_t*    get_mirror2();
    uint8_t*    get_ppu_reg();
    uint8_t*    get_ppu_reg_mirrors();
    uint8_t*    get_apu_io_regs();
    uint8_t*    get_apu_io_test_mode();
    uint8_t*    get_cartridge_space();
};
