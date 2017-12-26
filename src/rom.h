#pragma once

#include <cstdint>
#include <fstream>
#include <string>

using namespace std;

class ROM
{
private:
    uint8_t     prg_rom_pages;
    uint8_t     char_rom_pages;
    uint8_t     flags6;
    uint8_t     flags7;
    ifstream*   rom;
    
    bool read_header();
    
public:
    ROM(const string& fname);
    ~ROM();
    
    uint8_t get_prg_rom_pages();
    uint8_t get_char_rom_pages();
    uint8_t get_flags6();
    uint8_t get_flags7();
};

