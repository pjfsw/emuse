#include "condition.h"

bool checkCondition(uint16_t condition, M68kRegisters *registers) {
    bool c = getFlag(registers, SR_FLAGS_C);
    bool v = getFlag(registers, SR_FLAGS_V);
    bool z = getFlag(registers, SR_FLAGS_Z);
    bool n = getFlag(registers, SR_FLAGS_N);

    bool result = false;
    if (condition == 0) {  // TRUE
        result = true;
    } else if (condition == 1) {  // FALSE
        result = false;
    } else if (condition == 2) {  // BHI
        result = !c && !z;
    } else if (condition == 3) {  // BLS
        result = c || z;
    } else if (condition == 4) {  // BCC
        result = !c;
    } else if (condition == 5) {  // BCS
        result = c;
    } else if (condition == 6) {  // BNE
        result = !z;
    } else if (condition == 7) {  // BEQ
        result = z;
    } else if (condition == 8) {  // BVC
        result = !v;
    } else if (condition == 9) {  // BVS
        result = v;
    } else if (condition == 10) {  // BPL
        result = !n;
    } else if (condition == 11) {  // BMI
        result = n;
    } else if (condition == 12) {  // BGE
        result = n == v;
    } else if (condition == 13) {  // BLT
        result = n != v;
    } else if (condition == 14) {  // BGT
        result = !z && (n == v);
    } else if (condition == 15) {  // BLE
        result = z || (n != v);
    } else {
        return -1;
    }
    return result;
}
