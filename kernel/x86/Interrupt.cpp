#include"Interrupt.h"
#include"Log.h"
#include"stl/sidgen.h"
#include"cpu/CPUManager.h"
using namespace lr::sstl;

struct IDTItem
{
	unsigned int offsetl:16;
	unsigned int selector:16;
	unsigned int reserved1:8;
	unsigned int type:1;
	unsigned int reserved2:4;
	unsigned int dpl:2;
	unsigned int present:1;
	unsigned int offseth:16;
	void* GetHandler()
	{
		unsigned int a = 0;
		a = offseth;
		a<<= 16;
		a|=offsetl;
		return (void*)a;
	}
	IDTItem()
	{
		reserved1 = 0;
		reserved2 = 0b0111;
		present = 1;
	}
}__attribute__((packed));

IDTItem* Interrupt::table = nullptr; 

SINGLETON_CPP(Interrupt)
{
	this->table = (IDTItem*)&ItrnTable;	
	Assert(this->table);
	unsigned long* itable = &ItrnTable;
	for(int i = 0;i<256;i++)
	{
		IDTItem item = this->BuildIDTItem((InHandler_t)itable[i*2],0,8,IDT_TYPE_TRAP);
		this->table[i] = item;
	}
	this->irqArray.Initialize(new Map<int,IRQHandler>[256](),256);
}

void Interrupt::UniHandler(InterruptParams& params)
{
	Interrupt* ins = Interrupt::Instance();
	for(auto& irq : ins->irqArray[params.irqnum])
	{
		irq.second(params);
	}
	CPUManager::Instance()->EOI();
}
extern "C" void CHandler(InterruptParams params)
{
	if(params.irqnum != 66)
	{
		//LOG("IRQ: %d\n",params.irqnum);
	}
// 	if(params.irqnum == 14)MAGIC_DEBUG;
	if(params.irqnum <= 20 &&params.irqnum != 14
		&& params.irqnum != 7
	)
	{
		Interrupt::Cli();
		LOG("\nCrush!!! irqnum:%d\n eip:0x%x,esp:0x%x\n"
		"errorcode: 0x%x\n",
			params.irqnum,
		    params.eip,params.userEsp,
			params.errorCode
		);
		for(;;);
	}
	Interrupt::UniHandler(params);
}
void Interrupt::Initialize()
{
	Interrupt::LoadIDT();
}

int Interrupt::RegisterIRQ(IRQHandler _Han,IRQNum _Number)
{
	int id = idGen.GetID();
	irqArray[_Number].Insert(MakePair(id,_Han));
	return id;
}

bool Interrupt::UnregisterIRQ(int id)
{
	for(auto& i : irqArray)
	{
		auto tmp = i.Find(id);
		if(tmp!=i.End())
		{
			i.Erase(tmp);
			return true;
		}
	}
	return false;
}


void Interrupt::SetDPLOfIRQ(int IRQ,int dpl)
{
	this->table[IRQ].dpl = dpl;
}

IDTItem Interrupt::BuildIDTItem(InHandler_t _Handler
	,unsigned int dpl,unsigned int selector,unsigned int type)
{
	IDTItem item;
	addr_t h = (addr_t)_Handler;
	item.offsetl = h&0x0000ffff;
	item.offseth = (h>>16)&0x0000ffff;
	item.dpl = dpl;
	item.selector = selector;
	item.type = type;
	return item;
}