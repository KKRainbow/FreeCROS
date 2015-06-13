#include"Clock.h"
#include"HAL.h"
#include"Log.h"
#include"cpu/CPUManager.h"
const uint32_t TOTAL_FREQUENCE = 1193182;

SINGLETON_CPP(Clock)
{
	usCounter = 0;
}

void Clock::InitPIT()
{
	CPUManager::Instance()->RegisterIRQ(
			Clock::ClockHandler,HAL::IRQBase+2);
	SetPeriod(1800);	
	CPUManager::Instance()->GetHAL()->SetMaskOfIRQ(CLOCK_IRQ,false);	
}
int Clock::ClockHandler(InterruptParams& params)
{
	//先关闭中断,因为如果时钟过快会导致栈溢出
	uint32_t eflag;
	Clock* c = Clock::Instance();
	
	CPUManager::Instance()->GetHAL()->EOI();
	CPU* cpu = CPUManager::Instance()->GetCurrentCPU();
	//说明没有初始化完成呢.该cpu还未被加入CPUManager?
	if(cpu)
	{
		if(cpu->GetType() == CPU::Type::BSP)
		{
			c->SetCurrentCounter(c->GetCurrentCounter()+c->GetPeriod());
		}
		CPUManager::Instance()->ClockNotify();
	}
	return true;
}
uint32_t Clock::CalcReloadValOfPeriod(uint32_t _Us,uint32_t &res_Us)
{
	uint32_t fre = 1e6/_Us;
	uint16_t load = 0;
	uint32_t irq0_fractions,irq0_ms,irq0_frequency;

	__asm__ volatile("movl $0xffff,%%eax\n\t"
			"cmpl $18,%%ebx\n\t"
			"jbe __GotReloadVal\n\t"
			"movl $1,%%eax\n\t"
			"cmp $1193181,%%ebx\n\t"
			"jae __GotReloadVal\n\t"
			"movl $3579545,%%eax\n\t"
			"movl $0,%%edx\n\t"
			"divl %%ebx\n\t"
			"cmpl $(3579545/2),%%edx\n\t"
			"jb l1\n\t"
			"incl %%eax\n\t"
			"l1:\n\t"
			"movl $3,%%ebx\n\t"
			"movl $0,%%edx\n\t"
			"divl %%ebx\n\t"
			"cmpl $(3/2),%%edx\n\t"
			"jb l2\n\t"
			"incl %%eax\n\t"
			"l2:\n\t"
			"__GotReloadVal:\n\t"
			"movl %%eax,%%edi\n\t"
			"movw %%ax,%3\n\t"
			"movl %%eax,%%ebx\n\t"
			"movl $3579545,%%eax\n\t"
			"movl $0,%%edx\n\t"
			"divl %%ebx\n\t"
			"cmpl $(3579545/2),%%edx\n\t"
			"jb l3\n\t"
			"incl %%eax\n\t"
			"l3:\n\t"
			"movl $3,%%ebx\n\t"
			"movl $0,%%edx\n\t"
			"divl %%ebx\n\t"
			"cmpl $(3/2),%%edx\n\t"
			"jb l4\n\t"
			"incl %%eax\n\t"
			"l4:\n\t"
			"movl %%eax,%2\n\t"
			"\n\t"
			"movl %%edi, %%ebx\n\t"
			"movl $0xDBB3A062,%%eax\n\t"
			"mull %%ebx\n\t"
			"shrdl $10,%%edx,%%eax\n\t"
			"shrl $10,%%edx\n\t"
			"movl %%edx,%1\n\t"
			"movl %%eax,%0\n\t"
			"\n\t"
			:"=m"(irq0_fractions),"=m"(irq0_ms),"=m"(irq0_frequency),"=S"(load):"b"(fre));
	res_Us = irq0_ms;
	return load;
}
void Clock::SetPeriod(uint32_t _Us)
{
	union
	{
		struct
		{
			uint16_t low:8;
			uint16_t high:8;
		}val;
		uint16_t i;
	}tmp;
	tmp.i= CalcReloadValOfPeriod(_Us,currPeriod);
	currPeriod = _Us;
	Outb_p(0b00110100,0x43);	
	Outb_p(tmp.val.low,0x40);
	Outb_p(tmp.val.high,0x40);
}
uint64_t Clock::GetCurrentCounter()const
{
	return usCounter;
}
uint32_t Clock::GetPeriod()const
{
	return currPeriod;	
}
void Clock::SetCurrentCounter(uint64_t _Val)
{
	usCounter = _Val;
}
void Clock::KernelWait(uint32_t _Us)const
{
	uint32_t tmp = GetCurrentCounter();

	while(tmp+_Us > GetCurrentCounter())
	{
		Interrupt::Sti();
	}
}
