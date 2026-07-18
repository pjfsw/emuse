#include <stdio.h>

#include "decode.h"
#include "adda.h"
#include "alu.h"
#include "branch.h"
#include "btst.h"
#include "clr.h"
#include "jump.h"
#include "lea.h"
#include "move.h"
#include "movem.h"
#include "rte.h"
#include "rts.h"
#include "shift.h"
#include "sourcedest.h"
#include "tst.h"
#include "dbcc.h"
#include "div.h"

int getEffectiveAddress(M68kRegisters *registers, uint16_t mode, uint16_t reg, InstructionSize size,
    EffectiveAddress *ea, ReadWordFunc readWordFunc, void *readWriteUserdata) {

    ea->mode = mode;
    ea->xn = reg;

    if ((mode == AM_DREG) || (mode == AM_AREG)) { // d0, a0
        return 0;
    } else if ((mode == AM_ADDRESS) || (mode == AM_ADDRESS_POST_INC) || (mode == AM_ADDRESS_PRE_DEC)) { // (a0) or (a0)+
        ea->address = registers->a[reg];
        return 0;        
    } else if (mode == AM_ADDR_DISP) { // disp(a0)
        ea->displacement = (int16_t)readWordFunc(readWriteUserdata, registers->pc);
        increasePc(registers);
        ea->address = (int32_t)ea->displacement + registers->a[reg];
        return 4;
    } else if (mode == AM_ADDR_INDEX) { // disp(a0,d0)
        uint16_t extWord = readWordFunc(readWriteUserdata, registers->pc);
        increasePc(registers);

        ea->displacement  = (int16_t)(int8_t)(extWord & 255);
        uint16_t dispRegNo = (extWord >> 12) & 7;
        uint32_t *dispReg;

        ea->dispReg = dispRegNo;
        if (extWord & 0x8000) {
            dispReg = &registers->a[dispRegNo];            
            ea->addrRegDisp = true;
        } else {
            dispReg = &registers->d[dispRegNo];
            ea->addrRegDisp = false;
        }
        
        int32_t disp2;
        if (extWord & 0x800) {
            disp2 = (int32_t)*dispReg;
            ea->longRegDisp = true;
        } else {
            disp2 = (int32_t)(int16_t)(*dispReg & 0xffff);
            ea->longRegDisp = false;
        }
        ea->address = (int32_t)ea->displacement + disp2 + registers->a[reg];
        return 6;
    } else if (mode == AM_EXT) {
        if (reg == AM_EXT_IMMEDIATE) {
            // Immediate
            if (size == IS_LONG) {
                ea->immediate = ((uint32_t)readWordFunc(readWriteUserdata, registers->pc)) << 16;
                increasePc(registers);
                ea->immediate |= (uint32_t)readWordFunc(readWriteUserdata, registers->pc);
                increasePc(registers);
                return 8;
            } else {
                ea->immediate = readWordFunc(readWriteUserdata, registers->pc);
                if (size == IS_BYTE) {
                    ea->immediate = ea->immediate & 0xff;
                }
                increasePc(registers);
                return 4;
            }
        } else if (reg == AM_EXT_ABS_LONG) {
            ea->address = ((uint32_t)readWordFunc(readWriteUserdata, registers->pc)) << 16;
            increasePc(registers);
            ea->address |= (uint32_t)readWordFunc(readWriteUserdata, registers->pc);
            increasePc(registers);
            return 8;
        } else if (reg == AM_EXT_ABS_SHORT) {
            ea->address = (uint32_t)readWordFunc(readWriteUserdata, registers->pc);
            increasePc(registers);
            return 4;
        } else if (reg == AM_EXT_PC_DISP) {
            int16_t displacement = (int16_t)readWordFunc(readWriteUserdata, registers->pc);
            ea->address = registers->pc + (int32_t)displacement;
            increasePc(registers);
            return 4;
        }
    }
    return -1;
}

void setFlag(M68kRegisters *registers, uint16_t flag, uint16_t value) {
    registers->sr = (registers->sr & (0xffff-flag)) | (value * flag);
}

bool getFlag(M68kRegisters *registers, uint16_t flag) {
    return (registers->sr & flag) != 0;
}

void setNZ(M68kRegisters *registers, int32_t value) {
    setFlag(registers, SR_FLAGS_N, value < 0);
    setFlag(registers, SR_FLAGS_Z, value == 0);
}

typedef struct {
    uint16_t mask;
    uint16_t value;
    DecodeFunc decodeFunc;
    InstructionFamily family;
} DecodeRule;

// LSL !

static const DecodeRule rules[] = {
    { 0xffff, 0x4e75, decodeRts, IF_IMPLIED},
    { 0xffff, 0x4e73, decodeRte, IF_IMPLIED},
    { 0xffc0, 0x4e80, decodeJsr, IF_JUMP},
    { 0xffc0, 0x4ec0, decodeJmp, IF_JUMP},
    { 0xffc0, 0x46c0, decodeMoveToSr, IF_MOVE_TO_SR},
    { 0xffc0, 0x0800, decodeBtstImmediate, IF_MOVE}, 
    { 0xffc0, 0xe4c0, decodeRoxrEa, IF_MOVE},
    { 0xffc0, 0xe5c0, decodeRoxlEa, IF_MOVE},
    { 0xff00, 0x0600, decodeAddi, IF_MOVE },
    { 0xff00, 0x0000, decodeOri, IF_MOVE},
    { 0xff00, 0x0001, decodeAndi, IF_MOVE},
    { 0xff00, 0x4200, decodeClr, IF_SINGLE_DEST},
    { 0xff00, 0x4a00, decodeTst, IF_SINGLE_SRC},
    { 0xff00, 0x0c00, decodeCmpi, IF_MOVE },
    { 0xfb80, 0x4880, decodeMovem, IF_MOVEM}, 
    { 0xf118, 0xe008, decodeLsr, IF_MOVE},
    { 0xf118, 0xe108, decodeLsl, IF_MOVE},
    { 0xf118, 0xe010, decodeRoxr, IF_MOVE},
    { 0xf118, 0xe110, decodeRoxl, IF_MOVE},
    { 0xf118, 0xe018, decodeRor, IF_MOVE},
    { 0xf118, 0xe118, decodeRol, IF_MOVE},
    { 0xf1c0, 0x41c0, decodeLea, IF_LEA},
    { 0xf1c0, 0xb108, decodeCmpm, IF_MOVE },  // CMPM
    { 0xf1c0, 0xb0c0, decodeCmpa, IF_MOVE }, // CMPA.W
    { 0xf1c0, 0xb1c0, decodeCmpa, IF_MOVE }, // CMPA.L
    { 0xf1c0, 0x80c0, decodeDivu, IF_MOVE },
    { 0xf1c0, 0x81c0, decodeDivs, IF_MOVE },
    { 0xf130, 0xd100, decodeAddx, IF_MOVE },
    { 0xf130, 0xd108, decodeAddx, IF_MOVE },
    { 0xf0c8, 0x50c8, decodeDbcc, IF_DBCC },
    { 0xf0c0, 0xd0c0, decodeAdda, IF_MOVE },
    { 0xf0c0, 0x5000, decodeAddqSubq, IF_MOVE }, // size 00
    { 0xf0c0, 0x5040, decodeAddqSubq, IF_MOVE }, // size 01
    { 0xf0c0, 0x5080, decodeAddqSubq, IF_MOVE }, // size 10        
    { 0xf0c0, 0x5100, decodeAddqSubq, IF_MOVE }, // size 00
    { 0xf0c0, 0x5140, decodeAddqSubq, IF_MOVE }, // size 01
    { 0xf0c0, 0x5180, decodeAddqSubq, IF_MOVE }, // size 10        
    { 0xf000, 0xb000, decodeCmp, IF_MOVE  },  // CMP    
    { 0xf000, 0xd000, decodeAdd, IF_MOVE },
    { 0xf000, 0x9000, decodeSub, IF_MOVE },
    { 0xf000, 0xc000, decodeAnd, IF_MOVE },
    { 0xf000, 0x8000, decodeOr, IF_MOVE },
    { 0xf000, 0x1000, decodeMove, IF_MOVE },
    { 0xf000, 0x2000, decodeMove, IF_MOVE },
    { 0xf000, 0x3000, decodeMove, IF_MOVE },
    { 0xf000, 0x7000, decodeMoveq, IF_MOVE },
    { 0xf000, 0x6000, decodeBranch, IF_BRANCH },
};

int decode(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    uint16_t cycles = 0;
    uint16_t opcode = readWordFunc(readWriteUserdata, registers->pc);
    di->opcode = opcode;
    increasePc(registers);
    cycles += 4;    

    di->mnemonic = "???";

    for (int i = 0 ; i < sizeof(rules)/sizeof(DecodeRule); i++) {
        if ((opcode & rules[i].mask) == rules[i].value) {            
            int decodeCycles = rules[i].decodeFunc(opcode, di, registers, rwFunc, readWriteUserdata);
            if (decodeCycles < 0) {
                return 0;
            } else {
                di->family = rules[i].family;
                return cycles + decodeCycles;
            }
        }
    }
    // Not yet decoded instruction
    return 0;
}
