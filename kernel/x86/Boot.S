#include"../Multiboot.h"


.global boot_start,stack
.extern bspmain

.code32
.section ".boot"

.long (MULTIBOOT_HEADER_MAGIC)			//魔术头
.long (MULTIBOOT_HEADER_FLAGS)			
.long ~(MULTIBOOT_HEADER_FLAGS+MULTIBOOT_HEADER_MAGIC)+1	//checksum
//后面没有内容说明是采用elf的头而非自定义

.section ".text"

#define STACK_SIZE 16*1024
boot_start:
	cli
	pushl $boot_start
	movl $(stack+STACK_SIZE-4),%esp
	movl %esp,%ebp
	call next //保证栈中有东西,这句话可有课没有
next:
	pushl %ebp  /*栈的起始地址*/
	pushl %eax
	pushl %ebx

	call bspmain
die:
	hlt
	jmp die

.section ".bss"
	.comm stack,STACK_SIZE

	
.code16
.section .data
#define BA_START aps_boot_code_start
#define PIC(addr) $(addr-BA_START)
#define ABSOLUTE_ADDR(addr) (0x65000+addr-BA_START)

.global BA_START,aps_boot_code_end,aps_boot_data
BA_START:
	cli
	jmp 1f
	.align 4
	aps_boot_data:
		main_entry:
		.long 0		/*Main Entry*/ 
		stack_addr:
		.long 0
		stack_size:
		.long 0
		flag:
		.long 0
		flag_addr:
		.long 0
		zero_desc:
		.long 0
		.long 0
		cs_seg_desc:
		.long 0x0000ffff
		.long 0x00cf9a00
		ds_seg_desc:
		.long 0x0000ffff
		.long 0x00cf9200
		gdt_load_desc:
		.short 4*8
		gdt_load_desc_base:
		.long zero_desc-BA_START 
	1:
	xor %eax,%eax
	movw %cs,%ax

	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs
	movw %ax,%ss

	shll $4,%eax 
	movl %eax,%edx	 /*edx is the absolute addr of these code*/ 
	addl PIC(zero_desc),%eax
	movl PIC(gdt_load_desc_base),%ebx
	movl %eax,(%ebx)
	movl PIC(gdt_load_desc),%eax
	lgdt (%eax) 

	movl %cr0,%eax
	bts $0,%eax  /*Protected*/ 
	btr $2,%eax /*EM*/ 
	bts $1,%eax/*MP*/  
	bts $5,%eax  /*NE*/ 
	movl %eax,%cr0

	addl PIC(1f),%edx
	jmpl $0x08,$(ABSOLUTE_ADDR(1f))
	1:
	.code32

	movw $0x10,%ax
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs
	movw %ax,%ss

	.global tmp
	tmp:
	movl ABSOLUTE_ADDR(stack_addr),%eax
	movl %eax,%esp
	movl %eax,%ebp

	pushl %esp
	pushl ABSOLUTE_ADDR(stack_size)
	movl ABSOLUTE_ADDR(main_entry),%eax

	/*将flag置为0,已经初始化好了,记得要改的是原来地址的flag*/
	movl ABSOLUTE_ADDR(flag_addr),%ebx
	movl $0,(%ebx)

	call *%eax

	string:
	.ascii "Core is running \n"
aps_boot_code_end:

	