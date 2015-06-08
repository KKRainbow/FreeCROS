#include"Type.h"
#include"Multiboot.h"
#include"memory/MemoryManager.h"
#include"cpu/CPUManager.h"
#include"Log.h"
#include "thread/ThreadManager.h"
#include"ramdisk/RamDisk.h"
#include"misc/ServerLoader.h"

//全局变量声明
Multiboot globalMultiboot; //Mutiboot的所有信息都在这里获得
MemoryManager globalMemoryManager(true);
extern "C" void apmain(addr_t _StackAddr,size_t _StackSize)
{
	LOG("Core Running!\n",1);
	CPUManager::Instance()->GetCurrentCPU()->StartService();
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
	
	//TODO 开始写ramdisk把~~~~~哈哈哈,应该很简单,嗯= =
	//Ramdisk之后就可以加载Server,Server利用Ramdisk就可以完成设备初始化了
	//也就是我们可以开始写TTY的驱动程序了哈哈哈
	//加载Server!
	
	ServerLoader::Instance()->LoadModules();
	
	const size_t stackSize = 4*PAGE_SIZE;
	//m->InitAP((addr_t)apmain,stackSize);
	
	LOG("Start service!!!\n",1);
	m->GetCurrentCPU()->StartService();
	
	for(;;)
	{
		m->KernelWait(1e6);
		LOG("Core 1\n",1);
	}
	for(;;)__asm__("hlt");
}
