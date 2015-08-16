#include <errno.h>
#include"SystemCalls.h"
#include"memory/AddressSpaceManager.h"
#include"cpu/CPUManager.h"
#include"thread/ThreadManager.h"
#include"Log.h"
#include"ramdisk/RamDisk.h"

SYSCALL_METHOD_CPP(CreateThread)
{
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	//判断当前Thread是线程还是进程,如果是线程，那么新的线程的父线程理所当然是进程。
	Thread* newThread = ThreadManager::Instance()->CreateChildThread(curr);
	if(newThread->State()->Type() == ZOMBIE)
	{
		return -1;
	}
	else
	{
		newThread->GetCPUState().tss.regs.eax = 0;
		newThread->GetCPUState().tss.eip = _First;
		newThread->State()->ToReady(newThread);
		return newThread->GetPid();
	}
}

SYSCALL_METHOD_CPP(WriteToPhisicalAddr)
{
    ThreadManager::ReadDataFromCurrThread((void*)_First,(void*)_Sec,_Third);
	return 1;
}

SYSCALL_METHOD_CPP(ReadFromPhisicalAddr)
{
    ThreadManager::WriteDataToCurrThread((void*)_First,(void*)_Sec,_Third);
	return 1;
}

SYSCALL_METHOD_CPP(Log)
{
	char str[500];
	if(_Sec>=500)return -1;
    ThreadManager::ReadDataFromCurrThread(str,(void*)_First,_Sec);
	LOG(str,1);	
	return 1;
}

SYSCALL_METHOD_CPP(SendMessageTo)
{
	Message msg;
    ThreadManager::ReadDataFromCurrThread(&msg,(void*)_First,sizeof(Message));

	auto curr = CPUManager::Instance()->GetCurrentCPU()
		->GetCurrThreadRunning();
	if(_Sec & SEND_MESSAGE_FLAG_PROXY_PROCESS) {
		auto t = curr->belongTo ? curr->belongTo : curr;
		msg.source = t->GetPid();
	}
	else if (_Sec & SEND_MESSAGE_FLAG_PROXY_FATHER){
		auto t = curr->father ? curr->father : curr;
		msg.source = t->GetPid();
	}
	else
	{
		msg.source = curr->GetPid();
	}
	msg.timeStamp = CPUManager::Instance()->GetClockCounter();
	Thread* des = ThreadManager::Instance()->GetThreadByPID(msg.destination);
	if(des == nullptr)return -1;

	IPCMessage ipc(msg);

	while(!des->ReceiveMessage(ipc))
	{
		des->waitIPCSend.Wait();
	}

	des->waitIPCReceive.Wake();
	return 1;
}

SYSCALL_METHOD_CPP(ReceiveFrom)
{
	IPCMessage ipc;
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	while(!curr->ExtractMessage(_First,ipc))
	{
		curr->waitIPCReceive.Wait();
	}
	curr->waitIPCSend.Wake();
    ThreadManager::WriteDataToCurrThread((void*)_Sec,&ipc.msg,sizeof(ipc.msg));
	return 1;
}

SYSCALL_METHOD_CPP(ReceiveAll)
{
	IPCMessage ipc;
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	while(!curr->ExtractMessage(ipc))
	{
		curr->waitIPCReceive.Wait();	
	}
	curr->waitIPCSend.Wake();
    ThreadManager::WriteDataToCurrThread((void*)_First,&ipc.msg,sizeof(ipc.msg));
	return 1;
}

SYSCALL_METHOD_CPP(ReadDataFromThread)
{
	return 1;
}

static lr::sstl::MultiMap<irq_t,pid_t> irqMap;
static bool isInitialized = false;
static int RegisteredIRQHandler(InterruptParams& params)
{
	auto range = irqMap.EqualRange(params.irqnum);
	auto& b = range.first;
	auto& e = range.second;
	for(;b!=e;)
	{
		pid_t pid = b->second;
		auto t = ThreadManager::Instance()->GetThreadByPID(pid);
		if(t == nullptr)
		{
			auto tmp = b;
			b++;
			irqMap.Erase(tmp);
		}
		else
		{
			t->Kill(0,SIGINT);
			b++;
		}
	}
	return 1;
}
SYSCALL_METHOD_CPP(RegisterIRQ)//irqnum
{
	if(_First > 254)return -1;
	if(isInitialized == false)
	{
		new (&irqMap) decltype(irqMap)();
		isInitialized = true;
	}
	auto id = CPUManager::Instance()->RegisterIRQ((IRQHandler)RegisteredIRQHandler,_First);
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	irqMap.Insert(lr::sstl::MakePair((irq_t)_First,curr->GetPid()));
	return id;
}

SYSCALL_METHOD_CPP(RegisterChrDev) //devname
{
	auto rd = RamDisk::Instance();
	Assert(rd);
	
	char devname[500];
    ThreadManager::ReadDataFromCurrThread(devname,(void*)_First,sizeof(devname) - 1);
	devname[500] = 0;
	auto ri = rd->RegisterCharaterDevice(devname);
	return ri->GetID();
}

SYSCALL_METHOD_CPP(Open)//path
{
	auto rd = RamDisk::Instance();
	Message msg;
	char devname[500];
	
    ThreadManager::ReadDataFromCurrThread(devname,(void*)_First,sizeof(devname) - 1);
	devname[500] = 0;
	auto ri = rd->GetItemByPath(devname);
	if(ri == nullptr)return -1;

	auto res = ri->Open();
	//如果错误了就不用进行下面的步骤
	if(res < 0)return res;

    auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	auto fid = curr->GetNewFileSlot();
    if(fid < 0)return fid;
    File* file = curr->GetFileStruct(fid);
    Assert(file != nullptr);

    file->f_item = ri;
    return fid;
}

static const int SYSCALL_READ = 0;
static const int SYSCALL_WRITE = 1;
static const int SYSCALL_SEEK = 2;
static int ReadWrite(int _Id,char* _Buffer,size_t _Size,int _ReadWrite)
{
    auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();

    File* file = curr->GetFileStruct(_Id);
    if(!file)return -1;
	auto ri = file->f_item;
	if(ri == nullptr)
    {
        LOG("File struct without ramdisk item\n");
        return -1;
    }

	int res;
	if(_ReadWrite == SYSCALL_READ)
	{
		res = ri->Read(file, _Buffer, _Size);
	}
	else if(_ReadWrite == SYSCALL_WRITE)
	{
		res = ri->Write(file, _Buffer, _Size);
	}
    else if (_ReadWrite == SYSCALL_SEEK)
    {
        off_t offset = (off_t)_Buffer;
        int whence = _Size;
//        if (fd >= NR_OPEN || !(file=current->filp[fd]) || !(file->f_inode)
//            || !IS_SEEKABLE(MAJOR(file->f_inode->i_dev)))
//            return -EBADF;
//        if (file->f_inode->i_pipe)
//            return -ESPIPE;
        switch (whence) {
            case SEEK_SET:
                if (offset<0) return -EINVAL;
                file->f_pos=offset;
                break;
            case SEEK_CUR:
                if (file->f_pos+offset<0) return -EINVAL;
                file->f_pos += offset;
                break;
            case SEEK_END:
                int tmp;
                if ((tmp=file->f_item->GetSize()+offset) < 0)
                    return -EINVAL;
                file->f_pos = tmp;
                break;
            default:
                return -EINVAL;
        }
        ri->Seek(file, offset, whence);
        return file->f_pos;
    }
	else
	{
		Assert(false);
	}
	//如果错误了就不用进行下面的步骤
    return res;
}
SYSCALL_METHOD_CPP(Read)//path
{
	return ReadWrite(_First,(char*)_Sec,(size_t)_Third,SYSCALL_READ);
}

SYSCALL_METHOD_CPP(Write)//path
{
	return ReadWrite(_First,(char*)_Sec,(size_t)_Third,SYSCALL_WRITE);
}

SYSCALL_METHOD_CPP(Seek)
{
    return ReadWrite(_First,(char*)_Sec,(size_t)_Third,SYSCALL_SEEK);
}



SYSCALL_METHOD_CPP(Signal) //signum,handler,flag
{
	auto t = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(t);
	return t->AddSignalHandler(_First,_Sec,_Third);
}

SYSCALL_METHOD_CPP(Kill) //pid,signum
{
	auto t = ThreadManager::Instance()->GetThreadByPID(_First);
	if(t == nullptr)return -1;
	
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	
	if(t->Kill(curr->GetPid(),_Sec))
		return true;
	return false;
	
}

SYSCALL_METHOD_CPP(SignalRestore)//no params
{
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	if(curr&&curr->RestoreFromSignal())
	{
		CPUManager::Instance()->GetCurrentCPU()->ExhaustCurrThread();
		CPUManager::Instance()->GetCurrentCPU()->Run();
		//这个返回值其实没有意义
		return 1;
	}
	return -1;
}

SYSCALL_METHOD_CPP(Alarm) //time
{
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	curr->Alarm(_First * 1e3);
	return 0;
}


SYSCALL_METHOD_CPP(Pause) 
{
	WaitableObj wait;
	wait.Wait();
	return 0;
}
SYSCALL_METHOD_CPP(Sleep) 
{
	WaitableObj wait;
	wait.Sleep();
	return 0;
}

SYSCALL_METHOD_CPP(WakeUp)
{
	auto t = ThreadManager::Instance()->GetThreadByPID(_First);
	if(!t)return -1;
	if(t->State()->Type() == INTERRUPTABLE)
	{
		t->State()->ToReady(t);
	}
	return 0;
}

SYSCALL_METHOD_CPP(Exit)
{
 	//这里应该回收所有线程占用的资源
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	//先这么简单的实现，先实验tty的功能,便于写调试命令,
	//TODO 如果结束的是进程，那么还需要删掉所有的线程。
	ThreadManager::Instance()->RemoveThread(curr->GetPid());
	CPUManager::Instance()->GetCurrentCPU()->ExhaustCurrThread();
	CPUManager::Instance()->GetCurrentCPU()->Run();
}

SYSCALL_METHOD_CPP(GiveUp)
{
	CPUManager::Instance()->GetCurrentCPU()->ExhaustCurrThread();
	CPUManager::Instance()->GetCurrentCPU()->Run();
}