#pragma once

#include"Global.h"
#include"HAL.h" 
#include"Interrupt.h"

//8254 PIT控制
class Clock {
SINGLETON_H(Clock)

private:
    uint64_t usCounter;
    uint32_t currPeriod;

    static int ClockHandler(InterruptParams &params);

    uint32_t CalcReloadValOfPeriod(uint32_t _Us, uint32_t &res_Us);

    void SetCurrentCounter(uint64_t _Val);

public:
    static const int CLOCK_IRQ = HAL::IRQBase + 2;

    void InitPIT();

    void SetPeriod(uint32_t _Us);

    uint64_t GetCurrentCounter() const;

    uint32_t GetPeriod() const;

    void KernelWait(uint32_t _Us) const;
};
