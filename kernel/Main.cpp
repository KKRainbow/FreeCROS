#include"Type.h"
#include"Multiboot.h"
#include"memory/MemoryManager.h"
#include"cpu/CPUManager.h"
#include"Log.h"
#include "thread/ThreadManager.h"

//全局变量声明
Multiboot globalMultiboot; //Mutiboot的所有信息都在这里获得
MemoryManager globalMemoryManager(true);
extern "C" void apmain(addr_t _StackAddr,size_t _StackSize)
{
	LOG("Core Running!\n",1);
	for(;;)__asm__("hlt");
}


extern "C" int bspmain(MultibootInfo* multibootAddr,uint32_t magic)
{
	//设置全局SpinLock
	SpinLock::SetBasicMode(true);
	//验证Multiboot的有效性
  	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC||multibootAddr == 0)
	{
		return 0;
	} 
	new(&globalMultiboot)Multiboot();
	globalMultiboot.SetInfoAddr(multibootAddr);
	
	new(&globalMemoryManager)MemoryManager(true);
	
	auto m = CPUManager::Instance();
	m->Initialize();
	
	//CPUManager正常工作后,Spinlock可以正常使用了`
	SpinLock::SetBasicMode(false);
	
	LOG("Start service!!!\n",1);
	m->GetCurrentCPU()->StartService();
	
	const size_t stackSize = 4*PAGE_SIZE;
	m->InitAP((addr_t)apmain,stackSize);
	
	for(;;)
	{
		m->KernelWait(1e6);
		LOG("Core 1\n",1);
	}
	for(;;)__asm__("hlt");
}
