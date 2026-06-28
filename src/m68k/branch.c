#include "branch.h"

static int executeBranch(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = 6;
    bool c = getFlag(registers, SR_FLAGS_C);
    bool v = getFlag(registers, SR_FLAGS_V);
    bool z = getFlag(registers, SR_FLAGS_Z);
    bool n = getFlag(registers, SR_FLAGS_N);

    bool shouldBranch = true;
    if (di->condition == 0) { // BRA
        shouldBranch = true;
    } else if (di->condition == 1) { // BSR
        uint32_t address = registers->pc;
        registers->a[7] -= 4;
        rwFunc->ww(readWriteUserdata, registers->a[7], (uint16_t)(address >> 16));
        rwFunc->ww(readWriteUserdata, registers->a[7]+2, (uint16_t)address); 
        cycles += 8;
    } else if (di->condition == 2) { // BHI
        shouldBranch = !c && !z;
    } else if (di->condition == 3) { // BLS
        shouldBranch = c || z;
    } else if (di->condition == 4) { // BCC
        shouldBranch = !c;
    } else if (di->condition == 5) { // BCS
        shouldBranch = c;
    } else if (di->condition == 6) { // BNE
        shouldBranch = !z;
    } else if (di->condition == 7) { // BEQ
        shouldBranch = z;
    } else if (di->condition == 8) { // BVC
        shouldBranch = !v;
    } else if (di->condition == 9) { // BVS
        shouldBranch = v;
    } else if (di->condition == 10) { // BPL
        shouldBranch = !n;
    } else if (di->condition == 11) { // BMI
        shouldBranch = n;
    } else if (di->condition == 12) { // BGE
        shouldBranch = n == v;
    } else if (di->condition == 13) { // BLT
        shouldBranch = n != v;
    } else if (di->condition == 14) { // BGT
        shouldBranch = !z && (n == v);
    } else if (di->condition == 15) { // BLE
        shouldBranch = z || (n != v);
    } else {
        return -1;
    }

    if (shouldBranch) {
        registers->pc = align24(registers->pc + di->displacement);
    } else if (di->size == IS_WORD) {
        return 8;
    } else {
        return 4;
    }
    return cycles;  // We already counted the opcode fetch = 4 cycles
}

static char *mn_condition[] = {
    "BRA", "BSR", "BHI", "BLS", "BCC", "BCS", "BNE", "BEQ", "BVC", "BVS", "BPL", "BMI", "BGE", "BLT", "BGT", "BLE"};

int decodeBranch(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t condition = (opcode >> 8) & 15;
    di->condition = condition;
    di->mnemonic = mn_condition[condition];
    di->family = IF_BRANCH;
    di->execFunc = executeBranch;
    uint8_t displacement = opcode & 0xff;
    if (displacement == 0) {
        di->displacement = (int32_t)(int16_t)readWordFunc(readWriteUserdata, registers->pc) - 2;
        // !! Displacement is taken from the PC after the opcode word not after the extension word
        di->size = IS_WORD;
        increasePc(registers);
        //cycles += 4; Don't update cycles here because branch is annoying
    } else {
        di->displacement = (int8_t)displacement;
        di->size = IS_BYTE;
    }
    return 0;
}

