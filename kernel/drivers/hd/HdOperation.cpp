/*
 *  linux/kernel/hd.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * This is the low-level hd interrupt support. It traverses the
 * request-list, using interrupts to jump between functions. As
 * all the functions are called within interrupts, we may not
 * sleep. Special care is recommended.
 * 
 *  modified by Drew Eckhardt to check nr of hd's from the CMOS.
 */

#define MAJOR_NR 3

#include"Global.h"
#include <Multiboot.h>
#include <Interrupt.h>
#include <cpu/CPUManager.h>
#include <Clock.h>
#include <ramdisk/RamDisk.h>
#include <ramdisk/RamDiskItemKernel.h>
#include <drivers/buffer/BufferManager.h>
#include "BlockDev.h"
#include"HardDrive.h"
#include"hdreg.h"

using namespace lr::sstl;

#define CMOS_READ(addr) ({ \
Outb_p(0x80|addr,0x70); \
Inb_p(0x71); \
})

/* Max read/write errors/sector */
#define MAX_ERRORS	7
#define MAX_HD		2

static void recal_intr(void);
static void bad_rw_intr(void);

static int recalibrate = 0;
static int reset = 0;

static void (*DEVICE_INTR)(void) = nullptr;
Request* request;
static int DEVICE_TIMEOUT = 0;
void do_hd_request(Request* _ = nullptr);

/*
 *  This struct defines the HD's and their types.
 */
struct hd_i_struct {
	unsigned int head,sect,cyl,wpcom,lzone,ctl;
	};
#ifdef HD_TYPE
struct hd_i_struct hd_info[] = { HD_TYPE };
#define NR_HD ((sizeof (hd_info))/(sizeof (struct hd_i_struct)))
#else
struct hd_i_struct hd_info[] = { {0,0,0,0,0,0},{0,0,0,0,0,0} };
static int NR_HD = 0;
#endif

static struct hd_struct {
	long start_sect;
	long nr_sects;
	int dev_id;
	RamDiskItem* dev_item;
} hd[5*MAX_HD]={{0,0},};

static int hd_sizes[5*MAX_HD] = {0, };

#define port_read(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))

#define port_write(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))

/* This may be used only once, enforced by 'static int callable' */
int init_hd_info(char * BIOS)
{
	static int callable = 1;
	int i,drive;
	unsigned char cmos_disks;
	struct partition *p;
	struct Buffer * bh;

	if (!callable)
		return -1;
	callable = 0;

	const AString name[] = {
			"hda",
			"hdb"
	};

	for (drive=0 ; drive<2 ; drive++) {
		hd_info[drive].cyl = *(unsigned short *) BIOS;
		hd_info[drive].head = *(unsigned char *) (2+BIOS);
		hd_info[drive].wpcom = *(unsigned short *) (5+BIOS);
		hd_info[drive].ctl = *(unsigned char *) (8+BIOS);
		hd_info[drive].lzone = *(unsigned short *) (12+BIOS);
		hd_info[drive].sect = *(unsigned char *) (14+BIOS);
		BIOS += 16;
	}
	if (hd_info[1].cyl)
		NR_HD=2;
	else
		NR_HD=1;
	for (i=0 ; i<NR_HD ; i++) {
		hd[i*5].start_sect = 0;
		hd[i*5].nr_sects = hd_info[i].head*
				hd_info[i].sect*hd_info[i].cyl;

		RamDiskItemKernel* dev = new RamDiskItemKernel(0,RamDiskItem::Type::KERNELBLOCK,
													   name[i],i + 5,HdOpen,HdRead,HdWrite,nullptr);
		dev->SetSize(hd[5*i].nr_sects * 512);
		RamDisk::Instance()->CreateKernelDev(dev);
		hd[i+5].dev_id = i + 5;
		hd[i+5].dev_item = dev;
	}
	request = new Request(NR_REQUEST, DEVICE_REQUEST);

	if ((cmos_disks = CMOS_READ(0x12)) & 0xf0)
		if (cmos_disks & 0x0f)
			NR_HD = 2;
		else
			NR_HD = 1;
	else
		NR_HD = 0;
	for (i = NR_HD ; i < 2 ; i++) {
		hd[i*5].start_sect = 0;
		hd[i*5].nr_sects = 0;
	}

	for (drive=0 ; drive<NR_HD ; drive++) {
		if (!(bh = HdBlockRead(0, request, hd[drive+5].dev_item->GetID()))){
			LOG("Unable to read partition table of drive %d\n\r",
				drive);
			panic("");
		}
		if (bh->b_data[510] != 0x55 || (unsigned char)
		    bh->b_data[511] != 0xAA) {
			LOG("Bad partition table on drive %d\n\r",drive);
			panic("");
		}
		p = (partition*)(0x1BE + (char *)bh->b_data);
		for (i=1;i<5;i++,p++) {
			hd[i+5*drive].start_sect = p->start_sect;
			hd[i+5*drive].nr_sects = p->nr_sects;
			RamDiskItemKernel* dev = new RamDiskItemKernel(0,
														   RamDiskItem::Type::KERNELBLOCK,
														   name[drive] + ('0' + i),
														   i + 5 * drive,HdOpen,HdRead,HdWrite,nullptr);
			dev->SetSize(hd[i+5*drive].nr_sects * 512);
			RamDisk::Instance()->CreateKernelDev(dev);
		}
		BufferManager::Instance()->BufferRelease(bh);
	}

	for (i=0 ; i<5*MAX_HD ; i++)
		hd_sizes[i] = hd[i].nr_sects>>1 ;
	if (NR_HD)
		LOG("Partition table%s ok.\n\r",(NR_HD>1)?"s":"");
	return (0);
}

static int controller_ready(void)
{
	int retries = 100000;

//	while (--retries && (Inb_p(HD_STATUS)&0xc0)!=0x40);
	while (--retries && (Inb_p(HD_STATUS)&0x80));
	return (retries);
}

static int win_result(void)
{
	int i=Inb_p(HD_STATUS);

	if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
		== (READY_STAT | SEEK_STAT))
		return(0); /* ok */
	if (i&1) i=Inb(HD_ERROR);
	return (1);
}

static void hd_out(unsigned int drive,unsigned int nsect,unsigned int sect,
		unsigned int head,unsigned int cyl,unsigned int cmd,
		void (*intr_addr)(void))
{
	register int port asm("dx");

	if (drive>1 || head>15)
		panic("Trying to write bad sector");
	if (!controller_ready())
		panic("HD controller not ready");
	SET_INTR(intr_addr);
	Outb_p(hd_info[drive].ctl,HD_CMD);
	port=HD_DATA;
	Outb_p(hd_info[drive].wpcom>>2,++port);
	Outb_p(nsect,++port);
	Outb_p(sect,++port);
	Outb_p(cyl,++port);
	Outb_p(cyl>>8,++port);
	Outb_p(0xA0|(drive<<4)|head,++port);
	Outb(cmd,++port);
}

static int drive_busy(void)
{
	unsigned int i;
	unsigned char c;

	for (i = 0; i < 50000; i++) {
		c = Inb_p(HD_STATUS);
		c &= (BUSY_STAT | READY_STAT | SEEK_STAT);
		if (c == (READY_STAT | SEEK_STAT))
			return 0;
	}
	LOG("HD controller times out\n\r");
	return(1);
}

static void reset_controller(void)
{
	int	i;

	Outb(4,HD_CMD);
	for(i = 0; i < 1000; i++) __asm__("nop");
	Outb(hd_info[0].ctl & 0x0f ,HD_CMD);
	if (drive_busy())
		LOG("HD-controller still busy\n\r");
	if ((i = Inb(HD_ERROR)) != 1)
		LOG("HD-controller reset failed: %02x\n\r",i);
}

static void reset_hd(void)
{
	static int i;

repeat:
	if (reset) {
		reset = 0;
		i = -1;
		reset_controller();
	} else if (win_result()) {
		bad_rw_intr();
		if (reset)
			goto repeat;
	}
	i++;
	if (i < NR_HD) {
		hd_out(i,hd_info[i].sect,hd_info[i].sect,hd_info[i].head-1,
			hd_info[i].cyl,WIN_SPECIFY,&reset_hd);
	} else
		do_hd_request();
}

void unexpected_hd_interrupt(void)
{
	LOG("Unexpected HD interrupt\n\r");
	reset = 1;
	do_hd_request();
}

static void bad_rw_intr(void)
{
	if (++CURRENT->errors >= MAX_ERRORS)
		request->EndRequest(0);
	if (CURRENT->errors > MAX_ERRORS/2)
		reset = 1;
}

static void read_intr(void)
{
	if (win_result()) {
		bad_rw_intr();
		do_hd_request();
		return;
	}
	port_read(HD_DATA,CURRENT->buffer,256);
	CURRENT->errors = 0;
	CURRENT->buffer += 512;
	CURRENT->sector++;
	if (--CURRENT->nr_sectors) {
		SET_INTR(&read_intr);
		return;
	}
	request->EndRequest(1);
	do_hd_request();
}

static void write_intr(void)
{
	if (win_result()) {
		bad_rw_intr();
		do_hd_request();
		return;
	}
	if (--CURRENT->nr_sectors) {
		CURRENT->sector++;
		CURRENT->buffer += 512;
		SET_INTR(&write_intr);
		port_write(HD_DATA,CURRENT->buffer,256);
		return;
	}
	request->EndRequest(1);
	do_hd_request();
}

static void recal_intr(void)
{
	if (win_result())
		bad_rw_intr();
	do_hd_request();
}

void hd_times_out(void)
{
	if (!CURRENT)
		return;
	LOG("HD timeout");
	if (++CURRENT->errors >= MAX_ERRORS)
		request->EndRequest(0);
	SET_INTR(nullptr);
	reset = 1;
	do_hd_request();
}

void do_hd_request(Request* _Req)
{
	int i,r;
	unsigned int block,dev;
	unsigned int sec,head,cyl;
	unsigned int nsect;

	INIT_REQUEST;
	RamDiskItemKernel* item = (RamDiskItemKernel*)RamDisk::Instance()->GetItemByID(CURRENT->dev);
	dev = (unsigned int)(item ? item->GetDevnum() : 5*NR_HD + 10);
	block = CURRENT->sector;
	if (dev >= 5*NR_HD || block+2 > hd[dev].nr_sects) {
		request->EndRequest(0);
		goto repeat;
	}
	block += hd[dev].start_sect;
	dev /= 5;
	__asm__("divl %4":"=a" (block),"=d" (sec):"0" (block),"1" (0),
		"r" (hd_info[dev].sect));
	__asm__("divl %4":"=a" (cyl),"=d" (head):"0" (block),"1" (0),
		"r" (hd_info[dev].head));
	sec++;
	nsect = CURRENT->nr_sectors;
	if (reset) {
		recalibrate = 1;
		reset_hd();
		return;
	}
	if (recalibrate) {
		recalibrate = 0;
		hd_out(dev,hd_info[CURRENT_DEV].sect,0,0,0,
			WIN_RESTORE,&recal_intr);
		return;
	}	
	if (CURRENT->cmd == Req::WRITE) {
		hd_out(dev,nsect,sec,head,cyl,WIN_WRITE,&write_intr);
		for(i=0 ; i<10000 && !(r=Inb_p(HD_STATUS)&DRQ_STAT) ; i++)
			/* nothing */ ;
		if (!r) {
			bad_rw_intr();
			goto repeat;
		}
		port_write(HD_DATA,CURRENT->buffer,256);
	} else if (CURRENT->cmd == Req::READ) {
		hd_out(dev,nsect,sec,head,cyl,WIN_READ,&read_intr);
	} else
		panic("unknown hd-command");
}

int hd_time_intr(InterruptParams& _Params)
{
	if(hd_timeout)
	{
		if (--hd_timeout == 0)
		{
			hd_times_out();
		}
	}
	return 1;
}

int hd_interrupt(InterruptParams& _Param)
{
	if (DEVICE_INTR == nullptr)
	{
		unexpected_hd_interrupt();
	}
	else
	{
		DEVICE_INTR();
	}
	return 1;
}

void hd_init(void)
{
	uint16_t* intTable = (uint16_t*)globalMultiboot.GetIntTable();
	void* drive1 = (void*)(((int)intTable[0x41 * 2 + 1] << 4) + (int)intTable[0x41 * 2]);
	void* drive2 = (void*)(((int)intTable[0x46 * 2 + 1] << 4) + (int)intTable[0x46 * 2]);
	char* drive_info = new char[32];
	memcpy(drive_info, drive1, 16);
	memcpy(drive_info + 16, drive2, 16);
	CPUManager::Instance()->RegisterIRQ(hd_interrupt, HAL::IRQBase + 0x0E);
	CPUManager::Instance()->RegisterIRQ(hd_time_intr, Clock::CLOCK_IRQ);

	init_hd_info (drive_info);
}
