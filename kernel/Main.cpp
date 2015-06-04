#include"Multiboot.h"
#include"memory/MemoryManager.h"
#include"cpu/CPUManager.h"
#include"Log.h"
#include "thread/ThreadManager.h"

//全局变量声明
Multiboot globalMultiboot; //Mutiboot的所有信息都在这里获得
MemoryManager globalMemoryManager(true);

void test()
{
	for(;;)
	{
		CPUManager::Instance()->KernelWait(1000000);
		LOG("aaaa\n",1);
	}
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
	auto t = ThreadManager::Instance()->CreateThread(ThreadType::KERNEL);
	t->SetEntry((addr_t)test);
	t->State()->ToReady(t);
	
	m->GetCurrentCPU()->SetIdleThread(t);
	for(;;);
}
