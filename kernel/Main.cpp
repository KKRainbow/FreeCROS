#include"Multiboot.h"
#include"memory/MemoryManager.h"

//全局变量声明
Multiboot globalMultiboot; //Mutiboot的所有信息都在这里获得
MemoryManager globalMemoryManager;

extern "C" int bspmain(MultibootInfo* multibootAddr,uint32_t magic)
{
	//验证Multiboot的有效性
  	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC||multibootAddr == 0)
	{
		return 0;
	} 
	new(&globalMultiboot)Multiboot();
	new(&globalMemoryManager)MemoryManager();
}