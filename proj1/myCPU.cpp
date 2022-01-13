#include "myCPU.h"

#include "instruction.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

CPU::CPU(ifstream *file)
{
    memset(reg_, 0, sizeof(reg_));
    memset(dataMem_, 0, sizeof(dataMem_));

    newInstr_ = nullptr;
    instrID_ = nullptr;
    instrEX_ = nullptr;
    instrMEM_ = nullptr;
    instrWB_ = nullptr;

    loadInstructions(file);
}

CPU::~CPU()
{
    delete instrWB_;
    delete instrMEM_;
    delete instrEX_;
    delete instrID_;
    delete newInstr_;
}

// helper function for constructor to load instructions from file into instruction memory
void CPU::loadInstructions(ifstream *file)
{
    memset(instMem_, 0, sizeof(instMem_));

    string line;
    int i = 0;
    while (getline(*file, line) && i < maxInstBytes_)
    {
        uint8_t byte = (uint8_t)stoi(line);
        instMem_[i++] = byte;
    }
}

// fetch stage
void CPU::fetch(uint64_t PC)
{
    uint32_t instruction = 0;

    // read instruction from instruction memory.
    instruction = read4BytesInstMem(PC);

    newInstr_ = new Instruction(this, instruction);
}

// decode stage in cpu
void CPU::decode()
{

    if (!instrID_ || instrID_->getOpcode() == Instruction::ZERO)
        return;

    instrID_->decode();
}

// execute stage in cpu
void CPU::execute()
{
    if (!instrEX_ || instrEX_->getOpcode() == Instruction::ZERO)
        return;

    // execute ALU operations
    switch (instrEX_->getInstrType())
    {
    case Instruction::ADD:
        instrEX_->setAluOutput(reg_[instrEX_->getRs1()] + reg_[instrEX_->getRs2()]);
        break;
    case Instruction::LW:
    case Instruction::SW:
    case Instruction::ADDI:
        instrEX_->setAluOutput(reg_[instrEX_->getRs1()] + instrEX_->getImm());
        break;
    case Instruction::SUB:
        instrEX_->setAluOutput(reg_[instrEX_->getRs1()] - reg_[instrEX_->getRs2()]);
        break;
    case Instruction::OR:
        instrEX_->setAluOutput(reg_[instrEX_->getRs1()] | reg_[instrEX_->getRs2()]);
        break;
    case Instruction::ORI:
        instrEX_->setAluOutput(reg_[instrEX_->getRs1()] | instrEX_->getImm());
        break;
    case Instruction::AND:
        instrEX_->setAluOutput(reg_[instrEX_->getRs1()] & reg_[instrEX_->getRs2()]);
        break;
    case Instruction::ANDI:
        instrEX_->setAluOutput(reg_[instrEX_->getRs1()] & instrEX_->getImm());
        break;
    default:
        break;
    }
}

// memory stage in cpu
void CPU::memory()
{
    if (!instrMEM_ || instrMEM_->getOpcode() == Instruction::ZERO)
        return;

    // write or read depending on flags set.
    if (instrMEM_->getMemWr())
    {
        write4BytesDataMem(reg_[instrMEM_->getRs2()], instrMEM_->getAluOutput());
    }
    else if (instrMEM_->getMemRe())
    {
        instrMEM_->setDataFromMem(read4BytesDataMem(instrMEM_->getAluOutput()));
    }
}

// writeback stage in cpu
void CPU::writeback()
{
    if (!instrWB_ || instrWB_->getOpcode() == Instruction::ZERO)
        return;

    // write to register if flags are set
    if (instrWB_->getRegWrite())
    {
        if (instrWB_->getMemToReg())
        {
            reg_[instrWB_->getRd()] = instrWB_->getDataFromMem();
        }
        else
        {
            reg_[instrWB_->getRd()] = instrWB_->getAluOutput();
        }
    }
}

void CPU::cycle()
{
    delete instrWB_;
    instrWB_ = instrMEM_;
    instrMEM_ = instrEX_;
    instrEX_ = instrID_;
    instrID_ = newInstr_;
    newInstr_ = nullptr;
}

// load 4 Bytes from instruction memory into a uint32
uint32_t CPU::read4BytesInstMem(uint64_t location)
{
    if (location < 0 || location > maxInstBytes_ - 4)
        return 0;

    uint32_t data = 0;

    data |= instMem_[location + 3] << 24;
    data |= instMem_[location + 2] << 16;
    data |= instMem_[location + 1] << 8;
    data |= instMem_[location];

    return data;
}

// load 4 bytes from data memory into an int32
int32_t CPU::read4BytesDataMem(uint64_t location)
{
    if (location < 0 || location > maxDataBytes_ - 4)
        return 0;

    int32_t data = 0;

    data |= dataMem_[location + 3] << 24;
    data |= dataMem_[location + 2] << 16;
    data |= dataMem_[location + 1] << 8;
    data |= dataMem_[location];

    return data;
}

// write a int32 to data memory
void CPU::write4BytesDataMem(int32_t data, uint64_t location)
{
    if (location < 0 || location > maxDataBytes_ - 4)
        return;

    uint8_t byte = 0;

    // get each byte and write in little endian
    byte = data & 0xFF;
    dataMem_[location] = byte;

    byte = (data >> 8) & 0xFF;
    dataMem_[location + 1] = byte;

    byte = (data >> 16) & 0xFF;
    dataMem_[location + 2] = byte;

    byte = (data >> 24) & 0xFF;
    dataMem_[location + 3] = byte;
}

// helper function for CPU to sign extend
int16_t CPU::signExtend16(uint16_t num, int originalLength)
{
    if (originalLength >= 16)
        return (int16_t)num;

    bool padVal = num & (1 << (originalLength - 1));

    if (padVal == 0)
        return (int16_t)num;

    int16_t extended = 0xFFFF << originalLength;
    return extended | num;
}

// function to determine if we should end CPU loop
bool CPU::shouldEnd()
{
    int opcodeZeros = 0;

    if (!newInstr_ || newInstr_->getOpcode() == Instruction::ZERO)
        opcodeZeros++;
    if (!instrID_ || instrID_->getOpcode() == Instruction::ZERO)
        opcodeZeros++;
    if (!instrEX_ || instrEX_->getOpcode() == Instruction::ZERO)
        opcodeZeros++;
    if (!instrMEM_ || instrMEM_->getOpcode() == Instruction::ZERO)
        opcodeZeros++;
    if (!instrWB_ || instrWB_->getOpcode() == Instruction::ZERO)
        opcodeZeros++;

    if (opcodeZeros == 5)
        return true;

    return false;
}
