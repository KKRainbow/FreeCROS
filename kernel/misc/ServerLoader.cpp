#include"ServerLoader.h"
#include"Log.h"
#include"memory/MemoryManager.h"
#include"thread/ThreadManager.h"
#include"cpu/CPUManager.h"
#include"string.h"


SINGLETON_CPP(ServerLoader)
{

}
void ServerLoader::LoadModules()
{
	typedef MemoryManager::Module Module;
	if(globalMultiboot.TestFlagMods() == false)return;
	Module* mod = MemoryManager::Instance()->GetModules();
	
	size_t count = globalMultiboot.GetMultibootInfo().modsCount;
	ExecutableFormat* format;
	MemoryRegion* region = new MemoryRegion[14];
	AddressSpace* kas = AddressSpaceManager::Instance()->GetKernelAddressSpace();
	
	while(count--)
	{
		Thread* thread = ThreadManager::Instance()->CreateThread(SERVER);
		AddressSpace* space = thread->GetAddressSpace().Obj();
		format =  ExecutableFormat::findForMem((void*)mod->addr);	
		if(format == nullptr)
		{
			LOG("Unknown module format !\n",0);
			continue;
		}
		int i = format->regions(region,14);
		if(i<0)
		{
			LOG("Get memory region failed! Errorcode : %d\n",i);
		}
		//Load memory region into memory
		while(i--)
		{
			addr_t vmlower = PAGE_LOWER_ALIGN(region->virtualAddress);
			addr_t vmupper = PAGE_UPPER_ALIGN
			(region->virtualAddress+region->size+1);
			int pages = (vmupper - vmlower)>>PAGE_SHIFT;
			char* res = (char*)MemoryManager::Instance()
				->KernelPageAllocate(pages);
			Assert(res);
			addr_t tmpvir = vmlower;
			char* tmpphy = res;
			while(pages--)
			{
				space->MapVirtAddrToPhysAddr((addr_t)tmpphy,tmpvir);
				tmpvir += PAGE_SIZE;
				tmpphy += PAGE_SIZE;
			}
			AddressSpaceManager::Instance()->CopyDataFromAnotherSpace(*space,(void*)region[i].virtualAddress,
					*kas,region[i].data,vmupper-vmlower);
		}
		thread->SetEntry(format->entry());
		thread->State()->ToReady(thread);
		LOG("Thread Created, PID: %d\n",thread->GetPid());
		++mod;
	}
}
