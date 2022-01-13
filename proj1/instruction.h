#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include "myCPU.h"

// Mask to get opcode from raw instruction
static const uint32_t kOpcodeMask = 0b1111111;

class CPU;

class Instruction
{
public:
    // all opcodes
    enum OPCODE
    {
        ZERO = 0,
        R_TYPE = 0b0110011,
        I_TYPE = 0b0010011,
        LW_TYPE = 0b0000011,
        SW_TYPE = 0b0100011,
    };

    // instruction operations
    enum INSTR_TYPE
    {
        INVALID = -1,
        ADD = 0,
        ADDI = 1,
        SUB = 2,
        OR = 3,
        ORI = 4,
        AND = 5,
        ANDI = 6,
        LW = 7,
        SW = 8,
    };

    Instruction(CPU *cpu, uint32_t rawInstruction);

    // class static function to determine opcode
    static OPCODE determineOpcode(uint8_t rawOpcode);

    // Accessors
    OPCODE getOpcode() const { return opcode_; }
    INSTR_TYPE getInstrType() const { return instrType_; }
    uint8_t getRd() const { return rd_; }
    uint8_t getRs1() const { return rs1_; }
    uint8_t getRs2() const { return rs2_; }
    uint8_t getFunct3() const { return funct3_; }
    uint8_t getFunct7() const { return funct7_; }
    int16_t getImm() const { return imm_; }
    int32_t getAluOutput() const { return aluOutput_; }
    int32_t getDataFromMem() const { return dataFromMem_; }
    bool getRegWrite() const { return regWrite_; }
    bool getAluSrc() const { return aluSrc_; }
    bool getBranch() const { return branch_; }
    bool getMemRe() const { return memRe_; }
    bool getMemWr() const { return memWr_; }
    bool getMemToReg() const { return memToReg_; }

    // Setters
    void setAluOutput(int32_t val) { aluOutput_ = val; }
    void setDataFromMem(int32_t val) { dataFromMem_ = val; }

    // function to decode raw instruction
    void decode();

private:
    // reset all flags to false
    void resetFlags();
    // helper function for decode to set function type
    void setType();

    // Flags
    bool regWrite_;
    bool aluSrc_;
    bool branch_;
    bool memRe_;
    bool memWr_;
    bool memToReg_;

    CPU *cpu_;
    uint32_t rawInstruction_;

    OPCODE opcode_;
    INSTR_TYPE instrType_;
    int32_t aluOutput_;
    int32_t dataFromMem_;
    uint8_t rd_;
    uint8_t rs1_;
    uint8_t rs2_;
    uint8_t funct3_;
    uint8_t funct7_;
    int16_t imm_;
};

#endif