#include "rom.h"

#include <cstring>
#include <iostream>

/* iNES 2.0 */
const size_t HEADER_CONSTANT_SIZE               = 4;
const size_t HEADER_16KB_PRG_ROM_PAGES_SIZE     = 1;
const size_t HEADER_8KB_CHAR_ROM_PAGES_SIZE     = 1;
const size_t HEADER_FLAGS6_SIZE                 = 1;
const size_t HEADER_FLAGS7_SIZE                 = 1;
const size_t HEADER_8_15_CONSTANT_SIZE          = 8;

const uint8_t HEADER_CONSTANT[4]                = { 'N', 'E', 'S', 0x1A };
const uint8_t HEADER_8_15_CONSTANT[8]           = { 0 };


ROM::ROM(const string& fname) :
    rom(new ifstream())
{
    this->rom->open(fname, ios::binary);
    
    if (!this->rom->is_open()) {
        cerr << "ERROR: ROM file does not exist.\n";
    }
    
    if (!read_header()) {
        cerr << "ERROR: Invalid ROM format - failed header check.\n";
    }
}

ROM::~ROM()
{
    this->rom->close();
    delete this->rom;
}

bool ROM::read_header()
{
    uint8_t header_constant[HEADER_CONSTANT_SIZE];
    uint8_t header_8_15_constant[8];
    
    rom->read(reinterpret_cast<char*>(&header_constant), HEADER_CONSTANT_SIZE);
    rom->read(reinterpret_cast<char*>(&this->prg_rom_pages), HEADER_16KB_PRG_ROM_PAGES_SIZE);
    rom->read(reinterpret_cast<char*>(&this->char_rom_pages), HEADER_8KB_CHAR_ROM_PAGES_SIZE);
    rom->read(reinterpret_cast<char*>(&this->flags6), HEADER_FLAGS6_SIZE);
    rom->read(reinterpret_cast<char*>(&this->flags7), HEADER_FLAGS7_SIZE);
    rom->read(reinterpret_cast<char*>(&header_8_15_constant), HEADER_8_15_CONSTANT_SIZE);
    
    if (memcmp(header_constant, HEADER_CONSTANT, sizeof(header_constant)) != 0 ||
        memcmp(header_8_15_constant, HEADER_8_15_CONSTANT, sizeof(header_8_15_constant)) != 0)
    {
        return false;
    }
    
    return true;
}

uint8_t ROM::get_char_rom_pages()
{
    return this->char_rom_pages;
}

uint8_t ROM::get_prg_rom_pages()
{
    return this->prg_rom_pages;
}

uint8_t ROM::get_flags6()
{
    return this->flags6;
}

uint8_t ROM::get_flags7()
{
    return this->flags7;
}
