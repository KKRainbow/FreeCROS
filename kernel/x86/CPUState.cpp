#include"CPUState.h"

CPUState::CPUState(ThreadType _Type) {
    switch ( _Type ) {
        case SERVER:
            SetStateForServer();
            break;
        case KERNEL:
            SetStateForKernel();
            break;
        case USER:
            SetStateForUser();
            break;
    }
    return;
}

void CPUState::SetStateForKernel() {
    tss.ldt = USER_LDT_SEL;     //借用USER的,但是不采用
    tss.es = tss.ds = tss.fs = tss.gs = KERNEL_DATA_SEL;
    tss.cs = KERNEL_CODE_SEL;
    tss.regs.eflags = 0x200/*|0x100*/;
    tss.ss0 = KERNEL_DATA_SEL;
    tss.ss1 = tss.ss2 = tss.ss = KERNEL_DATA_SEL; //This is UNnecessary
    tss.esp0 = tss.esp1 = tss.esp2 = (uint32_t) kernelStack + sizeof(kernelStack) - 4;
}

void CPUState::SetStateForServer() {
    tss.ldt = SERVER_LDT_SEL;
    tss.es = tss.ds = tss.fs = tss.gs = SERVER_DATA_SEL;
    tss.cs = SERVER_CODE_SEL;
    tss.regs.eflags = 0x200/*|0x100*/;
    tss.ss0 = KERNEL_DATA_SEL;
    tss.ss1 = tss.ss2 = tss.ss = SERVER_DATA_SEL;
    tss.esp0 = tss.esp1 = tss.esp2 = (uint32_t) kernelStack + sizeof(kernelStack) - 4;
}

void CPUState::SetStateForUser() {
    tss.ldt = USER_LDT_SEL;
    tss.es = tss.ds = tss.fs = tss.gs = USER_DATA_SEL;
    tss.cs = USER_CODE_SEL;
    tss.regs.eflags = 0x200/*|0x100*/;
    tss.ss0 = KERNEL_DATA_SEL;
    tss.ss1 = tss.ss2 = tss.ss = USER_DATA_SEL;
    tss.esp0 = tss.esp1 = tss.esp2 = (uint32_t) kernelStack + sizeof(kernelStack) - 4;
}
