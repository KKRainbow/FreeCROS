#include"Segment.h"
#include"cpu/CPU.h"
#include"thread/Thread.h"

class CPUManager;

class CPUx86:public CPU
{
	private:
		struct GDT
		{
			const uint64_t Zero = 0;
			Segment kernelCode = Segment::BuildCodeSegment(0);
			Segment kernelData = Segment::BuildDataSegment(0);
			Segment previousTSS = Segment::BuildTSS(0);
			Segment currentTSS = Segment::BuildTSS(0);
			Segment	serverLDT = Segment::BuildLDT(0); 
			Segment	userLDT = Segment::BuildLDT(0); 
		}gdt;
		struct LDTServer
		{
			const uint64_t Zero = 0;
			Segment userCode = Segment::BuildCodeSegment(0,1);
			Segment userData = Segment::BuildDataSegment(0,1);
		}ldtServer;
		struct LDTUser
		{
			const uint64_t Zero = 0;
			Segment userCode = Segment::BuildCodeSegment(0,3);
			Segment userData = Segment::BuildDataSegment(0,3);
		}ldtUser;
		Thread idleThread;
		char stackAddr;
		static const int stackSize = 3*PAGE_SIZE;
		CPUManager* manager;
		Thread* currThread;
		uint64_t startCounter = 0;
		uint64_t endCounter = 0;  //两个判断当前进程时间片的Counter
		bool allDone = false;
		
		void InitPage();
		void InitGDT();
	public:
		static uint16_t GetGDTSelector(int i){return i<<3;}
		static uint16_t GetLDTSelector(int i){return (i<<3)|0b100;}
		virtual void Run()override;
		virtual ~CPUx86()override;
		CPUx86(CPU::Type _Type);
		virtual Thread* GetCurrThreadRunning()override;
		virtual Type GetType()override;
		virtual void StartService()override; //表明CPU可以接受线程了
		virtual void ExhaustCurrThread()override;
};
