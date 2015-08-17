#pragma once

#include"Global.h"
#include"Segment.h"
#include"Threads.h"

class CPUState {
private:
    struct I387Struct {
        uint32_t cwd;
        uint32_t swd;
        uint32_t twd;
        uint32_t fip;
        uint32_t fcs;
        uint32_t foo;
        uint32_t fos;
        uint32_t st_space[20]; /* 8*10 bytes for each FP-reg = 80 bytes */
    };

    void SetStateForKernel();

    void SetStateForServer();

    void SetStateForUser();

public:
    struct NormalRegs {
        uint32_t eflags;
        uint32_t eax, ecx, edx, ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
    }__attribute__((packed));
    struct TSSStruct {
        uint32_t back_link;
        /* 16 high bits zero */
        uint32_t esp0;
        uint32_t ss0;
        /* 16 high bits zero */
        uint32_t esp1;
        uint32_t ss1;
        /* 16 high bits zero */
        uint32_t esp2;
        uint32_t ss2;
        /* 16 high bits zero */
        uint32_t cr3;
        uint32_t eip;
        NormalRegs regs;
        //uint32_t eflages;
        //uint32_t eax, ecx, edx, ebx;
        //uint32_t esp;
        //uint32_t ebp;
        //uint32_t esi;
        //uint32_t edi;
        uint32_t es;
        /* 16 high bits zero */
        uint32_t cs;
        /* 16 high bits zero */
        uint32_t ss;
        /* 16 high bits zero */
        uint32_t ds;
        /* 16 high bits zero */
        uint32_t fs;
        /* 16 high bits zero */
        uint32_t gs;
        /* 16 high bits zero */
        uint32_t ldt;
        /* 16 high bits zero */
        uint32_t trace_bitmap;
        /* bits: trace 0, bitmap 16-31 */
        I387Struct i387;
    };
    struct LDT {
        uint64_t Zero = 0;
        Segment userCode = Segment::BuildCodeSegment(0, 1);
        Segment userData = Segment::BuildDataSegment(0, 1);
    };

    CPUState(ThreadType _Type = SERVER);

    char kernelStack[2 * PAGE_SIZE];
    TSSStruct tss;
    LDT ldt;

    static const uint16_t KERNEL_CODE_SEL = 0x08;
    static const uint16_t KERNEL_DATA_SEL = 8 + KERNEL_CODE_SEL;

    static const uint16_t USER_CODE_SEL = 0xf;
    static const uint16_t SERVER_CODE_SEL = 0xf - 2;
    static const uint16_t USER_DATA_SEL = 8 + USER_CODE_SEL;
    static const uint16_t SERVER_DATA_SEL = 8 + SERVER_CODE_SEL;

    static const uint16_t SERVER_LDT_SEL = 0x08 * 5;
    static const uint16_t USER_LDT_SEL = 0x08 * 6;
};
