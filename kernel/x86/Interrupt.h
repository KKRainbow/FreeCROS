#pragma once
#include"Global.h"
#include"stl/sarray.h"
#include"stl/smap.h"
#include"stl/stuple.h"
#include"stl/sidgen.h"
//Manage Interrupt table
extern unsigned long ItrnTable;
struct InterruptParams
{
	uint32_t isKernel;
	
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	
	uint32_t irqnum;
	uint32_t errorCode;
	
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t userEsp;
	uint32_t userSS;
};
typedef int (*IRQHandler)(InterruptParams& params);
typedef int IRQNum;
typedef lr::sstl::Tuple<uint32_t,uint32_t,uint32_t,uint32_t> SyscallParamPacket; 
extern "C" void CHandler(InterruptParams params);

struct IDTItem;
class Interrupt
{
	SINGLETON_H(Interrupt)
public:
	const int IDT_TYPE_INTR = 0;
	const int IDT_TYPE_TRAP = 1;
private:
	static IDTItem* table; 
	typedef void (*InHandler_t)();//这个类内部用这个类型的Handler
	IDTItem BuildIDTItem(InHandler_t _Handler
	,unsigned int dpl,unsigned int selector,unsigned int type);
private:
	friend void CHandler(InterruptParams params);
	static void UniHandler(InterruptParams& params);
public:
	static void Sti()
	{
		__asm__ __volatile__("sti\n\t");
	}
	static void Cli()
	{
		__asm__ __volatile__("cli\n\t");
	}
	int RegisterIRQ(IRQHandler _Han,IRQNum _Number);
	bool UnregisterIRQ(int id);
	void Initialize();
	void SetDPLOfIRQ(int IRQ,int dpl);
private:
	lr::sstl::IDGenerator<int> idGen;
	lr::sstl::Array<lr::sstl::Map<int,IRQHandler>> irqArray;
	static void LoadIDT()
	{
		struct
		{
			uint16_t limit = 4096;
			uint32_t base = (uint32_t)Interrupt::table;
		}__attribute__((packed))tmp;
		__asm__("lidt (%0)\n\t"::"a"(&tmp));	
	}
public:
	static void EnterCritical(uint32_t& eflag)
	{
		__asm__ __volatile__(
			"pushf\n\t"
			"movl (%%esp),%0\n\t"
			"addl $4,%%esp\n\t"
			"cli\n\t"
			:"=a"(eflag):);
	}
	static void LeaveCritical(uint32_t& eflag)
	{
		__asm__ __volatile__(
			"subl $4,%%esp\n\t"
			"movl %0,(%%esp)\n\t"
			"popf\n\t"
			::"a"(eflag));
	}
};
