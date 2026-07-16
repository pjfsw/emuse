#include "dbcc.h"
#include "condition.h"

static int executeDbcc(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    bool result = checkCondition(di->condition, registers);
    uint32_t *reg = &registers->d[di->src.xn];

    bool shouldBranch;
    int cycles;

    if (!result) {
        int16_t value = (int16_t)*reg;
        value = value - 1;        
        shouldBranch = (value != -1);
        *reg = (*reg & 0xffff0000) | (int32_t)(value);
        cycles = shouldBranch ? 6 : 10;
    } else {
        shouldBranch = false;
        cycles = 8;
    }
    if (shouldBranch) { 
        registers->pc = align24(registers->pc + di->displacement);
    }
    return cycles;
}

static char *mn_condition[] = {
    "DBT", "DBRA", "DBHI", "DBLS", "DBCC", "DBCS", "DBNE", "DBEQ", "DBVC", "DBVS", "DBPL", "DBMI", "DBGE", "DBLT", "DBGT", "DBLE"};

   
int decodeDbcc(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t condition = (opcode >> 8) & 15;
    di->condition = condition;
    di->mnemonic = mn_condition[condition];
    di->family = IF_BRANCH;
    di->execFunc = executeDbcc;
    uint16_t reg = opcode & 7;
    getEffectiveAddress(registers, AM_DREG, reg, IS_WORD, &di->src, readWordFunc, readWriteUserdata);
    di->displacement = (int32_t)(int16_t)readWordFunc(readWriteUserdata, registers->pc) - 2;
    // !! Displacement is taken from the PC after the opcode word not after the extension word
    di->size = IS_WORD;
    increasePc(registers);
    return 0;
}