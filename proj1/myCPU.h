#ifndef MYCPU_HPP
#define MYCPU_HPP

#include <stdint.h>
#include "instruction.h"
#include <fstream>

class Instruction;

class CPU
{
public:
    CPU(std::ifstream *file);
    ~CPU();

    // 5 pipeline stages of the CPU
    void fetch(uint64_t PC);
    void decode();
    void execute();
    void memory();
    void writeback();
    void cycle();

    // gets value of given register.
    int32_t getVal(uint8_t reg) { return reg_[reg]; }
    // sign extension
    int16_t signExtend16(uint16_t num, int originalLength);
    // function that determines if main loop should be broken.
    bool shouldEnd();

private:
    // helper function for constructor to load instructions into instMem_
    void loadInstructions(std::ifstream *file);

    // functions to read and write to inst/data memory
    uint32_t read4BytesInstMem(uint64_t location);
    int32_t read4BytesDataMem(uint64_t location);
    void write4BytesDataMem(int32_t data, uint64_t location);

    // Pointer to initialize new instruction
    Instruction *newInstr_;

    Instruction *instrID_;  // Represents IF/ID pipeline register.
    Instruction *instrEX_;  // Represents ID/EX pipeline register.
    Instruction *instrMEM_; // Represents EX/MEM pipeline register.
    Instruction *instrWB_;  // Represents MEM/RB pipeline register.

    // parameters for data sizes.
    static const int maxInstBytes_ = 4096;
    static const int maxDataBytes_ = 1024;
    static const int numRegisters_ = 32;

    uint8_t instMem_[maxInstBytes_]; // contains raw bytes of read instructions
    uint8_t dataMem_[maxDataBytes_]; // represents data memory in bytes (little endian)
    int32_t reg_[numRegisters_];     // holds register data (x0 - x31)
};

#endif