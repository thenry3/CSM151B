#include "instruction.h"

#include <iostream>
using namespace std;

Instruction::Instruction(CPU *cpu, uint32_t rawInstruction)
    : rd_(0),
      rs1_(0),
      rs2_(0),
      funct3_(0),
      funct7_(0),
      imm_(0),
      cpu_(cpu),
      instrType_(INVALID),
      regWrite_(false),
      aluSrc_(false),
      branch_(false),
      memRe_(false),
      memWr_(false),
      memToReg_(false),
      rawInstruction_(rawInstruction)
{
    resetFlags();

    opcode_ = determineOpcode(rawInstruction & kOpcodeMask);
    switch (opcode_)
    {
    case R_TYPE:
        regWrite_ = true;
        break;
    case I_TYPE:
        regWrite_ = true;
        aluSrc_ = true;
        break;
    case LW_TYPE:
        regWrite_ = true;
        aluSrc_ = true;
        memRe_ = true;
        memToReg_ = true;
        break;
    case SW_TYPE:
        aluSrc_ = true;
        memWr_ = true;
        break;
    default:
        break;
    }
}

// resetFlags resets all flags to false.
void Instruction::resetFlags()
{
    regWrite_ = false;
    aluSrc_ = false;
    branch_ = false;
    memRe_ = false;
    memWr_ = false;
    memToReg_ = false;
}

// decode parses the instruction based on opcode.
void Instruction::decode()
{
    switch (opcode_)
    {
    case R_TYPE:
        funct3_ = (rawInstruction_ >> 12) & 0b111;
        funct7_ = (rawInstruction_ >> 25) & 0b1111111;
        rd_ = (rawInstruction_ >> 7) & 0b11111;
        rs1_ = (rawInstruction_ >> 15) & 0b11111;
        rs2_ = (rawInstruction_ >> 20) & 0b11111;
        break;
    case I_TYPE:
    case LW_TYPE:
        funct3_ = (rawInstruction_ >> 12) & 0b111;
        rd_ = (rawInstruction_ >> 7) & 0b11111;
        rs1_ = (rawInstruction_ >> 15) & 0b11111;
        imm_ = cpu_->signExtend16((rawInstruction_ >> 20) & 0xFFF, 12);
        break;
    case SW_TYPE:
    {
        funct7_ = (rawInstruction_ >> 25) & 0b1111111;
        rs1_ = (rawInstruction_ >> 15) & 0b11111;
        rs2_ = (rawInstruction_ >> 20) & 0b11111;
        uint32_t imm7 = (rawInstruction_ >> 25) & 0b1111111;
        uint32_t imm5 = (rawInstruction_ >> 7) & 0b11111;
        imm_ = cpu_->signExtend16((imm7 << 5) | imm5, 12);
        break;
    }
    default:
        break;
    }

    setType();
}

void Instruction::setType()
{
    if (opcode_ == R_TYPE)
    {
        if (funct3_ == 0b000)
        {
            if (funct7_ == 0b0000000) // ADD
                instrType_ = ADD;
            else if (funct7_ == 0b0100000) // SUB
                instrType_ = SUB;
        }
        else if (funct3_ == 0b110) // OR
            instrType_ = OR;
        else if (funct3_ == 0b111) // AND
            instrType_ = AND;
    }
    else if (opcode_ == I_TYPE)
    {
        switch (funct3_)
        {
        case 0b000: // ADDI
            instrType_ = ADDI;
            break;
        case 0b110: // ORI
            instrType_ = ORI;
            break;
        case 0b111: // ANDI
            instrType_ = ANDI;
            break;
        }
    }
    else if (opcode_ == LW_TYPE)
    {
        instrType_ = LW;
    }
    else if (opcode_ == SW_TYPE)
    {
        instrType_ = SW;
    }
}

// Static function to determine opcode
Instruction::OPCODE Instruction::determineOpcode(uint8_t rawOpcode)
{
    switch (rawOpcode)
    {
    case 0b0110011:
        return R_TYPE;
        break;
    case 0b0010011:
        return I_TYPE;
        break;
    case 0b0000011:
        return LW_TYPE;
        break;
    case 0b0100011:
        return SW_TYPE;
        break;
    default:
        return ZERO;
    }
}